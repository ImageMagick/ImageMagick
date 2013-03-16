#  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building ImageMagick Coder library.

# Where coder modules get installed
codersdir = $(CODER_PATH)

if DPS_DELEGATE
MAGICK_DPS_MODULES = coders/dps.la
MAGICK_DPS_SRCS = coders/dps.c
endif

if DJVU_DELEGATE
MAGICK_DJVU_MODULES = coders/djvu.la
MAGICK_DJVU_SRCS = coders/djvu.c
endif

if OPENEXR_DELEGATE
MAGICK_EXR_MODULES = coders/exr.la
MAGICK_EXR_SRCS = coders/exr.c
endif

if FPX_DELEGATE
MAGICK_FPX_MODULES = coders/fpx.la
MAGICK_FPX_SRCS = coders/fpx.c
endif

if JBIG_DELEGATE
MAGICK_JBIG_MODULES = coders/jbig.la
MAGICK_JBIG_SRCS = coders/jbig.c
endif

if JPEG_DELEGATE
MAGICK_JPEG_MODULES = coders/jpeg.la
MAGICK_JPEG_SRCS = coders/jpeg.c
endif

if JP2_DELEGATE
MAGICK_JP2_MODULES = coders/jp2.la
MAGICK_JP2_SRCS = coders/jp2.c
endif

if PNG_DELEGATE
MAGICK_PNG_MODULES = coders/png.la
MAGICK_PNG_SRCS = coders/png.c
endif

if TIFF_DELEGATE
MAGICK_TIFF_MODULES = coders/ept.la coders/tiff.la
MAGICK_TIFF_SRCS = coders/ept.c coders/tiff.c
endif

if WEBP_DELEGATE
MAGICK_WEBP_MODULES = coders/webp.la
MAGICK_WEBP_SRCS = coders/webp.c
endif

if WINGDI32_DELEGATE
MAGICK_GDI32_MODULES = coders/clipboard.la coders/emf.la
MAGICK_GDI32_SRCS = coders/clipboard.c coders/emf.c
endif

if WMF_DELEGATE
MAGICK_WMF_MODULES = coders/wmf.la
MAGICK_WMF_SRCS = coders/wmf.c
endif

if X11_DELEGATE
MAGICK_X11_MODULES = coders/x.la coders/xwd.la
MAGICK_X11_SRCS = coders/x.c coders/xwd.c
endif

MAGICK_CODER_SRCS = \
	coders/aai.c \
	coders/art.c \
	coders/avs.c \
	coders/bgr.c \
	coders/bmp.c \
	coders/braille.c \
	coders/cals.c \
	coders/caption.c \
	coders/cin.c \
	coders/cip.c \
	coders/clip.c \
	coders/cmyk.c \
	coders/cut.c \
	coders/dcm.c \
	coders/dds.c \
	coders/debug.c \
	coders/dib.c \
	coders/dng.c \
	coders/dot.c \
	coders/dpx.c \
	coders/fax.c \
	coders/fd.c \
	coders/fits.c \
	coders/gif.c \
	coders/gradient.c \
	coders/gray.c \
	coders/hald.c \
	coders/hdr.c \
	coders/histogram.c \
	coders/hrz.c \
	coders/html.c \
	coders/jnx.c \
	coders/icon.c \
	coders/info.c \
	coders/inline.c \
	coders/ipl.c \
	coders/label.c \
	coders/mac.c \
	coders/magick.c \
	coders/map.c \
	coders/mat.c \
	coders/matte.c \
	coders/meta.c \
	coders/miff.c \
	coders/mono.c \
	coders/mpc.c \
	coders/mpeg.c \
	coders/mpr.c \
	coders/msl.c \
	coders/mtv.c \
	coders/mvg.c \
	coders/null.c \
	coders/otb.c \
	coders/palm.c \
	coders/pango.c \
	coders/pattern.c \
	coders/pcd.c \
	coders/pcl.c \
	coders/pcx.c \
	coders/pdb.c \
	coders/pdf.c \
	coders/pes.c \
	coders/pict.c \
	coders/pix.c \
	coders/plasma.c \
	coders/pnm.c \
	coders/preview.c \
	coders/ps.c \
	coders/ps2.c \
	coders/ps3.c \
	coders/psd.c \
	coders/pwp.c \
	coders/raw.c \
	coders/rgb.c \
	coders/rla.c \
	coders/rle.c \
	coders/scr.c \
	coders/sct.c \
	coders/sfw.c \
	coders/sgi.c \
	coders/stegano.c \
	coders/sun.c \
	coders/svg.c \
	coders/tga.c \
	coders/thumbnail.c \
	coders/tile.c \
	coders/tim.c \
	coders/ttf.c \
	coders/txt.c \
	coders/uil.c \
	coders/url.c \
	coders/uyvy.c \
	coders/vicar.c \
	coders/vid.c \
	coders/viff.c \
	coders/wbmp.c \
	coders/wpg.c \
	coders/xbm.c \
	coders/xc.c \
	coders/xcf.c \
	coders/xpm.c \
	coders/xps.c \
	coders/ycbcr.c \
	coders/yuv.c \
  $(MAGICK_DPS_SRCS) \
  $(MAGICK_DJVU_SRCS) \
  $(MAGICK_EXR_SRCS) \
  $(MAGICK_FPX_SRCS) \
  $(MAGICK_GDI32_SRCS) \
  $(MAGICK_JBIG_SRCS) \
  $(MAGICK_JPEG_SRCS) \
  $(MAGICK_JP2_SRCS) \
  $(MAGICK_PNG_SRCS) \
  $(MAGICK_TIFF_SRCS) \
  $(MAGICK_WEBP_SRCS) \
  $(MAGICK_WMF_SRCS) \
  $(MAGICK_X11_SRCS)

if WITH_MODULES
coders_LTLIBRARIES = \
	coders/aai.la \
	coders/art.la \
	coders/avs.la \
	coders/bgr.la \
	coders/bmp.la \
	coders/braille.la \
	coders/cals.la \
	coders/caption.la \
	coders/cin.la \
	coders/cip.la \
	coders/clip.la \
	coders/cmyk.la \
	coders/cut.la \
	coders/dcm.la \
	coders/dds.la \
	coders/debug.la \
	coders/dib.la \
	coders/dng.la \
	coders/dot.la \
	coders/dpx.la \
	coders/fax.la \
	coders/fd.la \
	coders/fits.la \
	coders/gif.la \
	coders/gradient.la \
	coders/gray.la \
	coders/hald.la \
	coders/hdr.la \
	coders/histogram.la \
	coders/hrz.la \
	coders/html.la \
	coders/jnx.la \
	coders/icon.la \
	coders/info.la \
	coders/inline.la \
	coders/ipl.la \
	coders/label.la \
	coders/mac.la \
	coders/magick.la \
	coders/map.la \
	coders/mat.la \
	coders/matte.la \
	coders/meta.la \
	coders/miff.la \
	coders/mono.la \
	coders/mpc.la \
	coders/mpeg.la \
	coders/mpr.la \
	coders/msl.la \
	coders/mtv.la \
	coders/mvg.la \
	coders/null.la \
	coders/otb.la \
	coders/palm.la \
	coders/pango.la \
	coders/pattern.la \
	coders/pcd.la \
	coders/pcl.la \
	coders/pcx.la \
	coders/pdb.la \
	coders/pdf.la \
	coders/pes.la \
	coders/pict.la \
	coders/pix.la \
	coders/plasma.la \
	coders/pnm.la \
	coders/preview.la \
	coders/ps.la \
	coders/ps2.la \
	coders/ps3.la \
	coders/psd.la \
	coders/pwp.la \
	coders/raw.la \
	coders/rgb.la \
	coders/rla.la \
	coders/rle.la \
	coders/scr.la \
	coders/sct.la \
	coders/sfw.la \
	coders/sgi.la \
	coders/stegano.la \
	coders/sun.la \
	coders/svg.la \
	coders/tga.la \
	coders/thumbnail.la \
	coders/tile.la \
	coders/tim.la \
	coders/ttf.la \
	coders/txt.la \
	coders/uil.la \
	coders/url.la \
	coders/uyvy.la \
	coders/vicar.la \
	coders/vid.la \
	coders/viff.la \
	coders/wbmp.la \
	coders/wpg.la \
	coders/xbm.la \
	coders/xc.la \
	coders/xcf.la \
	coders/xpm.la \
	coders/xps.la \
	coders/ycbcr.la \
	coders/yuv.la \
  $(MAGICK_DPS_MODULES) \
  $(MAGICK_DJVU_MODULES) \
  $(MAGICK_EXR_MODULES) \
  $(MAGICK_FPX_MODULES) \
  $(MAGICK_GDI32_MODULES)  \
  $(MAGICK_JBIG_MODULES) \
  $(MAGICK_JPEG_MODULES) \
  $(MAGICK_JP2_MODULES) \
  $(MAGICK_PNG_MODULES) \
  $(MAGICK_TIFF_MODULES) \
  $(MAGICK_WEBP_MODULES) \
  $(MAGICK_WMF_MODULES) \
  $(MAGICK_X11_MODULES)
else
coders_LTLIBRARIES =
endif # WITH_MODULES

# AAI coder module
coders_aai_la_SOURCES      = coders/aai.c
coders_aai_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_aai_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_aai_la_LIBADD       = $(MAGICKCORE_LIBS)

# ART coder module
coders_art_la_SOURCES      = coders/art.c
coders_art_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_art_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_art_la_LIBADD       = $(MAGICKCORE_LIBS)

# AVS coder module
coders_avs_la_SOURCES      = coders/avs.c
coders_avs_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_avs_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_avs_la_LIBADD       = $(MAGICKCORE_LIBS)

# BRG coder module
coders_bgr_la_SOURCES      = coders/bgr.c
coders_bgr_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_bgr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_bgr_la_LIBADD       = $(MAGICKCORE_LIBS)

# BMP coder module
coders_bmp_la_SOURCES      = coders/bmp.c
coders_bmp_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_bmp_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_bmp_la_LIBADD       = $(MAGICKCORE_LIBS)

# BRAILLE coder module
coders_braille_la_SOURCES  = coders/braille.c
coders_braille_la_CPPFLAGS = $(MODULE_EXTRA_CPPFLAGS)
coders_braille_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_braille_la_LIBADD   = $(MAGICKCORE_LIBS)

# CALS coder module
coders_cals_la_SOURCES  = coders/cals.c
coders_cals_la_CPPFLAGS = $(MODULE_EXTRA_CPPFLAGS)
coders_cals_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_cals_la_LIBADD   = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# CAPTION coder module
coders_caption_la_SOURCES  = coders/caption.c
coders_caption_la_CPPFLAGS = $(MODULE_EXTRA_CPPFLAGS)
coders_caption_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_caption_la_LIBADD   = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# CINEON coder module
coders_cin_la_SOURCES      = coders/cin.c
coders_cin_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_cin_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cin_la_LIBADD       = $(MAGICKCORE_LIBS)

# CIP coder module
coders_cip_la_SOURCES      = coders/cip.c
coders_cip_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_cip_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cip_la_LIBADD       = $(MAGICKCORE_LIBS)

# CLIP coder module
coders_clip_la_SOURCES     = coders/clip.c
coders_clip_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_clip_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_clip_la_LIBADD      = $(MAGICKCORE_LIBS)

# CLIPBOARD coder module
coders_clipboard_la_SOURCES= coders/clipboard.c
coders_clipboard_la_CPPFLAGS= $(MODULE_EXTRA_CPPFLAGS)
coders_clipboard_la_LDFLAGS= $(MODULECOMMONFLAGS)
coders_clipboard_la_LIBADD = $(MAGICKCORE_LIBS) $(GDI32_LIBS)

# CMYK coder module
coders_cmyk_la_SOURCES     = coders/cmyk.c
coders_cmyk_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_cmyk_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_cmyk_la_LIBADD      = $(MAGICKCORE_LIBS)

# CUT coder module
coders_cut_la_SOURCES      = coders/cut.c
coders_cut_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_cut_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cut_la_LIBADD       = $(MAGICKCORE_LIBS)

# DCM coder module
coders_dcm_la_SOURCES      = coders/dcm.c
coders_dcm_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dcm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dcm_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# DDS coder module
coders_dds_la_SOURCES      = coders/dds.c
coders_dds_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dds_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dds_la_LIBADD       = $(MAGICKCORE_LIBS)

# DEBUG coder module
coders_debug_la_SOURCES      = coders/debug.c
coders_debug_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_debug_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_debug_la_LIBADD       = $(MAGICKCORE_LIBS)

# DIB coder module
coders_dib_la_SOURCES      = coders/dib.c
coders_dib_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dib_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dib_la_LIBADD       = $(MAGICKCORE_LIBS)

# DJVU coder module
coders_djvu_la_SOURCES     = coders/djvu.c
coders_djvu_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_djvu_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_djvu_la_LIBADD      = $(MAGICKCORE_LIBS) $(DJVU_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# DNG coder module
coders_dng_la_SOURCES      = coders/dng.c
coders_dng_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dng_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dng_la_LIBADD       = $(MAGICKCORE_LIBS)

# DPS coder module
coders_dps_la_SOURCES      = coders/dps.c
coders_dps_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dps_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dps_la_LIBADD       = $(MAGICKCORE_LIBS) $(DPS_LIBS) $(XEXT_LIBS) $(X11_LIBS)

# DPX coder module
coders_dpx_la_SOURCES      = coders/dpx.c
coders_dpx_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_dpx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dpx_la_LIBADD       = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# DOT coder module
coders_dot_la_SOURCES      = coders/dot.c
coders_dot_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS) $(GVC_CFLAGS)
coders_dot_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dot_la_LIBADD       = $(MAGICKCORE_LIBS) $(GVC_LIBS)

# EMF coder module
coders_emf_la_SOURCES      = coders/emf.c
coders_emf_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_emf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_emf_la_LIBADD       = $(MAGICKCORE_LIBS) $(GDI32_LIBS)

# EPT coder module
coders_ept_la_SOURCES      = coders/ept.c
coders_ept_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_ept_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ept_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# EXR coder module
coders_exr_la_SOURCES      = coders/exr.c
coders_exr_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS) $(OPENEXR_CFLAGS)
coders_exr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_exr_la_LIBADD       = $(MAGICKCORE_LIBS) $(OPENEXR_LIBS) $(MATH_LIBS)

# FAX coder module
coders_fax_la_SOURCES      = coders/fax.c
coders_fax_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_fax_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_fax_la_LIBADD       = $(MAGICKCORE_LIBS)

# FD coder module
coders_fd_la_SOURCES      = coders/fd.c
coders_fd_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_fd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_fd_la_LIBADD       = $(MAGICKCORE_LIBS)

# FITS coder module
coders_fits_la_SOURCES     = coders/fits.c
coders_fits_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_fits_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_fits_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# FPX coder module
coders_fpx_la_SOURCES      = coders/fpx.c
coders_fpx_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_fpx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_fpx_la_LIBADD       = $(MAGICKCORE_LIBS) $(FPX_LIBS) -lstdc++

# GIF coder module
coders_gif_la_SOURCES      = coders/gif.c
coders_gif_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_gif_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_gif_la_LIBADD       = $(MAGICKCORE_LIBS)

# GRAY coder module
coders_gray_la_SOURCES     = coders/gray.c
coders_gray_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_gray_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_gray_la_LIBADD      = $(MAGICKCORE_LIBS)

# GRADIENT coder module
coders_gradient_la_SOURCES = coders/gradient.c
coders_gradient_la_CPPFLAGS= $(MODULE_EXTRA_CPPFLAGS)
coders_gradient_la_LDFLAGS = $(MODULECOMMONFLAGS)
coders_gradient_la_LIBADD  = $(MAGICKCORE_LIBS)

# HISTOGRAM coder module
coders_histogram_la_SOURCES = coders/histogram.c
coders_histogram_la_CPPFLAGS= $(MODULE_EXTRA_CPPFLAGS)
coders_histogram_la_LDFLAGS = $(MODULECOMMONFLAGS)
coders_histogram_la_LIBADD  = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# HALD coder module
coders_hald_la_SOURCES     = coders/hald.c
coders_hald_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_hald_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_hald_la_LIBADD      = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# HDR coder module
coders_hdr_la_SOURCES     = coders/hdr.c
coders_hdr_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_hdr_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_hdr_la_LIBADD      = $(MAGICKCORE_LIBS) $(GOMP_LIBS) $(MATH_LIBS)

# HRZ coder module
coders_hrz_la_SOURCES      = coders/hrz.c
coders_hrz_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_hrz_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_hrz_la_LIBADD       = $(MAGICKCORE_LIBS)

# HTML coder module
coders_html_la_SOURCES     = coders/html.c
coders_html_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_html_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_html_la_LIBADD      = $(MAGICKCORE_LIBS)

# JNX coder module
coders_jnx_la_SOURCES     = coders/jnx.c
coders_jnx_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_jnx_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_jnx_la_LIBADD      = $(MAGICKCORE_LIBS)

# ICON coder module
coders_icon_la_SOURCES     = coders/icon.c
coders_icon_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_icon_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_icon_la_LIBADD      = $(MAGICKCORE_LIBS)

# INFO coder module
coders_info_la_SOURCES     = coders/info.c
coders_info_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_info_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_info_la_LIBADD      = $(MAGICKCORE_LIBS)

# INLINE coder module
coders_inline_la_SOURCES      = coders/inline.c
coders_inline_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_inline_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_inline_la_LIBADD       = $(MAGICKCORE_LIBS)

# IPL coder module
coders_ipl_la_SOURCES     = coders/ipl.c
coders_ipl_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_ipl_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_ipl_la_LIBADD      = $(MAGICKCORE_LIBS)

# JBIG coder module
coders_jbig_la_SOURCES     = coders/jbig.c
coders_jbig_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_jbig_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_jbig_la_LIBADD      = $(MAGICKCORE_LIBS) $(JBIG_LIBS)

# JPEG coder module
coders_jpeg_la_SOURCES     = coders/jpeg.c
coders_jpeg_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_jpeg_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_jpeg_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS) $(JPEG_LIBS)

# JPEG 2000 coder module
coders_jp2_la_SOURCES      = coders/jp2.c
coders_jp2_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_jp2_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_jp2_la_LIBADD       = $(MAGICKCORE_LIBS) $(JP2_LIBS) $(JPEG_LIBS) $(MATH_LIBS)

# LABEL coder module
coders_label_la_SOURCES    = coders/label.c
coders_label_la_CPPFLAGS   = $(MODULE_EXTRA_CPPFLAGS)
coders_label_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_label_la_LIBADD     = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# MAC coder module
coders_mac_la_SOURCES      = coders/mac.c
coders_mac_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mac_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mac_la_LIBADD       = $(MAGICKCORE_LIBS)

# MAGICK coder module
coders_magick_la_SOURCES   = coders/magick.c
coders_magick_la_CPPFLAGS  = $(MODULE_EXTRA_CPPFLAGS)
coders_magick_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_magick_la_LIBADD    = $(MAGICKCORE_LIBS)

# MAP coder module
coders_map_la_SOURCES      = coders/map.c
coders_map_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_map_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_map_la_LIBADD       = $(MAGICKCORE_LIBS)

# MAT coder module
coders_mat_la_SOURCES      = coders/mat.c
coders_mat_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mat_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mat_la_LIBADD       = $(MAGICKCORE_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# MATTE coder module
coders_matte_la_SOURCES    = coders/matte.c
coders_matte_la_CPPFLAGS   = $(MODULE_EXTRA_CPPFLAGS)
coders_matte_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_matte_la_LIBADD     = $(MAGICKCORE_LIBS)

# META coder module
coders_meta_la_SOURCES     = coders/meta.c
coders_meta_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_meta_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_meta_la_LIBADD      = $(MAGICKCORE_LIBS)

# MIFF coder module
coders_miff_la_SOURCES     = coders/miff.c
coders_miff_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_miff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_miff_la_LIBADD      = $(MAGICKCORE_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(BZLIB_LIBS)

# MONO coder module
coders_mono_la_SOURCES     = coders/mono.c
coders_mono_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_mono_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_mono_la_LIBADD      = $(MAGICKCORE_LIBS)

# MPC coder module
coders_mpc_la_SOURCES      = coders/mpc.c
coders_mpc_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mpc_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mpc_la_LIBADD       = $(MAGICKCORE_LIBS)

# MPEG coder module
coders_mpeg_la_SOURCES     = coders/mpeg.c
coders_mpeg_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_mpeg_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_mpeg_la_LIBADD      = $(MAGICKCORE_LIBS)

# MPR coder module
coders_mpr_la_SOURCES      = coders/mpr.c
coders_mpr_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mpr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mpr_la_LIBADD       = $(MAGICKCORE_LIBS)

# MSL coder module
coders_msl_la_SOURCES      = coders/msl.c
coders_msl_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_msl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_msl_la_LIBADD       = $(MAGICKCORE_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# MTV coder module
coders_mtv_la_SOURCES      = coders/mtv.c
coders_mtv_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mtv_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mtv_la_LIBADD       = $(MAGICKCORE_LIBS)

# MVG coder module
coders_mvg_la_SOURCES      = coders/mvg.c
coders_mvg_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_mvg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mvg_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# NULL coder module
coders_null_la_SOURCES     = coders/null.c
coders_null_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_null_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_null_la_LIBADD      = $(MAGICKCORE_LIBS)

# OTB coder module
coders_otb_la_SOURCES      = coders/otb.c
coders_otb_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_otb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_otb_la_LIBADD       = $(MAGICKCORE_LIBS)

# PALM coder module
coders_palm_la_SOURCES     = coders/palm.c
coders_palm_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_palm_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_palm_la_LIBADD      = $(MAGICKCORE_LIBS)

# PANGO coder module
coders_pango_la_SOURCES    = coders/pango.c
coders_pango_la_CPPFLAGS   = $(MODULE_EXTRA_CPPFLAGS) $(PANGO_CFLAGS)
coders_pango_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_pango_la_LIBADD     = $(MAGICKCORE_LIBS) $(PANGO_LIBS)

# Pattern module
coders_pattern_la_SOURCES   = coders/pattern.c
coders_pattern_la_CPPFLAGS  = $(MODULE_EXTRA_CPPFLAGS)
coders_pattern_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_pattern_la_LIBADD    = $(MAGICKCORE_LIBS)

# PCD coder module
coders_pcd_la_SOURCES      = coders/pcd.c
coders_pcd_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pcd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcd_la_LIBADD       = $(MAGICKCORE_LIBS)

# PCL coder module
coders_pcl_la_SOURCES      = coders/pcl.c
coders_pcl_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pcl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcl_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# PCX coder module
coders_pcx_la_SOURCES      = coders/pcx.c
coders_pcx_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pcx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcx_la_LIBADD       = $(MAGICKCORE_LIBS)

# PDB coder module
coders_pdb_la_SOURCES      = coders/pdb.c
coders_pdb_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pdb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pdb_la_LIBADD       = $(MAGICKCORE_LIBS)

# PDF coder module
coders_pdf_la_SOURCES      = coders/pdf.c
coders_pdf_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pdf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pdf_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(GS_LIBS) $(MATH_LIBS)

# PES coder module
coders_pes_la_SOURCES      = coders/pes.c
coders_pes_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pes_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pes_la_LIBADD       = $(MAGICKCORE_LIBS)

# PICT coder module
coders_pict_la_SOURCES     = coders/pict.c
coders_pict_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_pict_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_pict_la_LIBADD      = $(MAGICKCORE_LIBS)

# PIX coder module
coders_pix_la_SOURCES      = coders/pix.c
coders_pix_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pix_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pix_la_LIBADD       = $(MAGICKCORE_LIBS)

# PNG coder module
coders_png_la_SOURCES      = coders/png.c
coders_png_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_png_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_png_la_LIBADD       = $(MAGICKCORE_LIBS) $(PNG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# PLASMA coder module
coders_plasma_la_SOURCES   = coders/plasma.c
coders_plasma_la_CPPFLAGS  = $(MODULE_EXTRA_CPPFLAGS)
coders_plasma_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_plasma_la_LIBADD    = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# PNM coder module
coders_pnm_la_SOURCES      = coders/pnm.c
coders_pnm_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pnm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pnm_la_LIBADD       = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# PREVIEW coder module
coders_preview_la_SOURCES  = coders/preview.c
coders_preview_la_CPPFLAGS = $(MODULE_EXTRA_CPPFLAGS)
coders_preview_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_preview_la_LIBADD   = $(MAGICKCORE_LIBS)

# PS coder module
coders_ps_la_SOURCES       = coders/ps.c
coders_ps_la_CPPFLAGS      = $(MODULE_EXTRA_CPPFLAGS)
coders_ps_la_LDFLAGS       = $(MODULECOMMONFLAGS)
coders_ps_la_LIBADD        = $(MAGICKCORE_LIBS) $(GS_LIBS) $(MATH_LIBS)

# PS2 coder module
coders_ps2_la_SOURCES      = coders/ps2.c
coders_ps2_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_ps2_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ps2_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# PS3 coder module
coders_ps3_la_SOURCES      = coders/ps3.c
coders_ps3_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_ps3_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ps3_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# PSD coder module
coders_psd_la_SOURCES      = coders/psd.c
coders_psd_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_psd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_psd_la_LIBADD       = $(MAGICKCORE_LIBS)

# PWP coder module
coders_pwp_la_SOURCES      = coders/pwp.c
coders_pwp_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_pwp_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pwp_la_LIBADD       = $(MAGICKCORE_LIBS)

# RAW coder module
coders_raw_la_SOURCES      = coders/raw.c
coders_raw_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_raw_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_raw_la_LIBADD       = $(MAGICKCORE_LIBS)

# RGB coder module
coders_rgb_la_SOURCES      = coders/rgb.c
coders_rgb_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_rgb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rgb_la_LIBADD       = $(MAGICKCORE_LIBS)

# RLA coder module
coders_rla_la_SOURCES      = coders/rla.c
coders_rla_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_rla_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rla_la_LIBADD       = $(MAGICKCORE_LIBS)

# RLE coder module
coders_rle_la_SOURCES      = coders/rle.c
coders_rle_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_rle_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rle_la_LIBADD       = $(MAGICKCORE_LIBS)

# SCT coder module
coders_sct_la_SOURCES      = coders/sct.c
coders_sct_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_sct_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sct_la_LIBADD       = $(MAGICKCORE_LIBS)

# SFW coder module
coders_sfw_la_SOURCES      = coders/sfw.c
coders_sfw_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_sfw_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sfw_la_LIBADD       = $(MAGICKCORE_LIBS)

# SGI coder module
coders_sgi_la_SOURCES      = coders/sgi.c
coders_sgi_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_sgi_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sgi_la_LIBADD       = $(MAGICKCORE_LIBS)

# STEGANO coder module
coders_stegano_la_SOURCES  = coders/stegano.c
coders_stegano_la_CPPFLAGS = $(MODULE_EXTRA_CPPFLAGS)
coders_stegano_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_stegano_la_LIBADD   = $(MAGICKCORE_LIBS)

# SUN coder module
coders_sun_la_SOURCES      = coders/sun.c
coders_sun_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_sun_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sun_la_LIBADD       = $(MAGICKCORE_LIBS)

# SCR coder module
coders_scr_la_SOURCES      = coders/scr.c
coders_scr_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_scr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_scr_la_LIBADD       = $(MAGICKCORE_LIBS)

# SVG coder module
coders_svg_la_SOURCES      = coders/svg.c
coders_svg_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS) $(RSVG_CFLAGS) $(CAIRO_SVG_CFLAGS)
coders_svg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_svg_la_LIBADD       = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(AUTOTRACE_LIBS) $(RSVG_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# TGA coder module
coders_tga_la_SOURCES      = coders/tga.c
coders_tga_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_tga_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_tga_la_LIBADD       = $(MAGICKCORE_LIBS)

# THUMBNAIL coder module
coders_thumbnail_la_SOURCES= coders/thumbnail.c
coders_thumbnail_la_CPPFLAGS= $(MODULE_EXTRA_CPPFLAGS)
coders_thumbnail_la_LDFLAGS= $(MODULECOMMONFLAGS)
coders_thumbnail_la_LIBADD = $(MAGICKCORE_LIBS)

# TIFF coder module
coders_tiff_la_SOURCES     = coders/tiff.c
coders_tiff_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_tiff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_tiff_la_LIBADD      = $(MAGICKCORE_LIBS) $(TIFF_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# TILE coder module
coders_tile_la_SOURCES     = coders/tile.c
coders_tile_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_tile_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_tile_la_LIBADD      = $(MAGICKCORE_LIBS)

# TIM coder module
coders_tim_la_SOURCES      = coders/tim.c
coders_tim_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_tim_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_tim_la_LIBADD       = $(MAGICKCORE_LIBS)

# TTF coder module
coders_ttf_la_SOURCES      = coders/ttf.c
coders_ttf_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_ttf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ttf_la_LIBADD       = $(MAGICKCORE_LIBS)

# TXT coder module
coders_txt_la_SOURCES      = coders/txt.c
coders_txt_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_txt_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_txt_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# UIL coder module
coders_uil_la_SOURCES      = coders/uil.c
coders_uil_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_uil_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_uil_la_LIBADD       = $(MAGICKCORE_LIBS)

# URL coder module
coders_url_la_SOURCES      = coders/url.c
coders_url_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_url_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_url_la_LIBADD       = $(MAGICKCORE_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# UYVY coder module
coders_uyvy_la_SOURCES     = coders/uyvy.c
coders_uyvy_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_uyvy_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_uyvy_la_LIBADD      = $(MAGICKCORE_LIBS)

# VICAR coder module
coders_vicar_la_SOURCES    = coders/vicar.c
coders_vicar_la_CPPFLAGS   = $(MODULE_EXTRA_CPPFLAGS)
coders_vicar_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_vicar_la_LIBADD     = $(MAGICKCORE_LIBS)

# VID coder module
coders_vid_la_SOURCES      = coders/vid.c
coders_vid_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_vid_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_vid_la_LIBADD       = $(MAGICKCORE_LIBS)

# VIFF coder module
coders_viff_la_SOURCES     = coders/viff.c
coders_viff_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_viff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_viff_la_LIBADD      = $(MAGICKCORE_LIBS)

# WBMP coder module
coders_wbmp_la_SOURCES     = coders/wbmp.c
coders_wbmp_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_wbmp_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_wbmp_la_LIBADD      = $(MAGICKCORE_LIBS)

# WEBP coder module
coders_webp_la_SOURCES      = coders/webp.c
coders_webp_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_webp_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_webp_la_LIBADD       = $(MAGICKCORE_LIBS) $(WEBP_LIBS) $(MATH_LIBS)

# WMF coder module
coders_wmf_la_SOURCES      = coders/wmf.c
coders_wmf_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_wmf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_wmf_la_LIBADD       = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(WMF_LIBS) $(WMF_LIBS_DEPS) $(MATH_LIBS)

# WPG coder module
coders_wpg_la_SOURCES      = coders/wpg.c
coders_wpg_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_wpg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_wpg_la_LIBADD       = $(MAGICKCORE_LIBS)

# X coder module
coders_x_la_SOURCES        = coders/x.c
coders_x_la_CPPFLAGS       = $(MODULE_EXTRA_CPPFLAGS)
coders_x_la_LDFLAGS        = $(MODULECOMMONFLAGS)
coders_x_la_LIBADD         = $(MAGICKCORE_LIBS) $(X11_LIBS)

# XBM coder module
coders_xbm_la_SOURCES      = coders/xbm.c
coders_xbm_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_xbm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xbm_la_LIBADD       = $(MAGICKCORE_LIBS)

# XC coder module
coders_xc_la_SOURCES       = coders/xc.c
coders_xc_la_CPPFLAGS      = $(MODULE_EXTRA_CPPFLAGS)
coders_xc_la_LDFLAGS       = $(MODULECOMMONFLAGS)
coders_xc_la_LIBADD        = $(MAGICKCORE_LIBS)

# XCF coder module
coders_xcf_la_SOURCES      = coders/xcf.c
coders_xcf_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_xcf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xcf_la_LIBADD       = $(MAGICKCORE_LIBS)

# XPM coder module
coders_xpm_la_SOURCES      = coders/xpm.c
coders_xpm_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_xpm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xpm_la_LIBADD       = $(MAGICKCORE_LIBS)

# XPS coder module
coders_xps_la_SOURCES      = coders/xps.c
coders_xps_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_xps_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xps_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# XWD coder module
coders_xwd_la_SOURCES      = coders/xwd.c
coders_xwd_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_xwd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xwd_la_LIBADD       = $(MAGICKCORE_LIBS) $(X11_LIBS)

# YCBCR coder module
coders_ycbcr_la_SOURCES     = coders/ycbcr.c
coders_ycbcr_la_CPPFLAGS    = $(MODULE_EXTRA_CPPFLAGS)
coders_ycbcr_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_ycbcr_la_LIBADD      = $(MAGICKCORE_LIBS)

# YUV coder module
coders_yuv_la_SOURCES      = coders/yuv.c
coders_yuv_la_CPPFLAGS     = $(MODULE_EXTRA_CPPFLAGS)
coders_yuv_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_yuv_la_LIBADD       = $(MAGICKCORE_LIBS)

#  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
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
MAGICKCORE_DPS_MODULES = coders/dps.la
MAGICKCORE_DPS_SRCS = coders/dps.c
endif

if DJVU_DELEGATE
MAGICKCORE_DJVU_MODULES = coders/djvu.la
MAGICKCORE_DJVU_SRCS = coders/djvu.c
endif

if OPENEXR_DELEGATE
MAGICKCORE_EXR_MODULES = coders/exr.la
MAGICKCORE_EXR_SRCS = coders/exr.c
endif

if FLIF_DELEGATE
MAGICKCORE_FLIF_MODULES = coders/flif.la
MAGICKCORE_FLIF_SRCS = coders/flif.c
endif

if FPX_DELEGATE
MAGICKCORE_FPX_MODULES = coders/fpx.la
MAGICKCORE_FPX_SRCS = coders/fpx.c
endif

if HEIC_DELEGATE
MAGICKCORE_HEIC_MODULES = coders/heic.la
MAGICKCORE_HEIC_SRCS = coders/heic.c
endif

if JBIG_DELEGATE
MAGICKCORE_JBIG_MODULES = coders/jbig.la
MAGICKCORE_JBIG_SRCS = coders/jbig.c
endif

if JPEG_DELEGATE
MAGICKCORE_JPEG_MODULES = coders/jpeg.la
MAGICKCORE_JPEG_SRCS = coders/jpeg.c
endif

if JXL_DELEGATE
MAGICKCORE_JXL_MODULES = coders/jxl.la
MAGICKCORE_JXL_SRCS = coders/jxl.c
endif

if DMR_DELEGATE
MAGICKCORE_DMR_MODULES = coders/dmr.la
MAGICKCORE_DMR_SRCS = coders/dmr.c
endif

if LIBOPENJP2_DELEGATE
MAGICKCORE_JP2_MODULES = coders/jp2.la
MAGICKCORE_JP2_SRCS = coders/jp2.c
endif

if PNG_DELEGATE
MAGICKCORE_PNG_MODULES = coders/png.la
MAGICKCORE_PNG_SRCS = coders/png.c
endif

if TIFF_DELEGATE
MAGICKCORE_TIFF_MODULES = coders/ept.la coders/tiff.la
MAGICKCORE_TIFF_SRCS = coders/ept.c coders/tiff.c
endif

if UHDR_DELEGATE
MAGICKCORE_UHDR_MODULES = coders/uhdr.la
MAGICKCORE_UHDR_SRCS = coders/uhdr.c
endif

if WEBP_DELEGATE
MAGICKCORE_WEBP_MODULES = coders/webp.la
MAGICKCORE_WEBP_SRCS = coders/webp.c
endif

if WINGDI32_DELEGATE
MAGICKCORE_GDI32_MODULES = coders/clipboard.la coders/emf.la
MAGICKCORE_GDI32_SRCS = coders/clipboard.c coders/emf.c
endif

if WMF_DELEGATE
MAGICKCORE_WMF_MODULES = coders/wmf.la
MAGICKCORE_WMF_SRCS = coders/wmf.c
endif

if X11_DELEGATE
MAGICKCORE_X11_MODULES = coders/x.la coders/xwd.la
MAGICKCORE_X11_SRCS = coders/x.c coders/xwd.c
endif

MAGICK_CODER_CPPFLAGS = \
	$(MODULECOMMONCPPFLAGS) \
	$(MODULE_EXTRA_CPPFLAGS)

MAGICKCORE_CODER_SRCS = \
	coders/aai.c \
	coders/aai.h \
	coders/art.c \
	coders/art.h \
	coders/ashlar.c \
	coders/ashlar.h \
	coders/avs.c \
	coders/avs.h \
	coders/bayer.c \
	coders/bayer.h \
	coders/bgr.c \
	coders/bgr.h \
	coders/bmp.c \
	coders/bmp.h \
	coders/braille.c \
	coders/braille.h \
	coders/bytebuffer-private.h \
	coders/cals.c \
	coders/cals.h \
	coders/caption.c \
	coders/caption.h \
	coders/cin.c \
	coders/cin.h \
	coders/cip.c \
	coders/cip.h \
	coders/clipboard.h \
	coders/clip.c \
	coders/clip.h \
	coders/cmyk.c \
	coders/cmyk.h \
	coders/coders.h \
	coders/coders-list.h \
	coders/coders-private.h \
	coders/cube.c \
	coders/cube.h \
	coders/cut.c \
	coders/cut.h \
	coders/dcm.c \
	coders/dcm.h \
	coders/dds.c \
	coders/dds.h \
	coders/debug.c \
	coders/debug.h \
	coders/dib.c \
	coders/dib.h \
	coders/djvu.h \
	coders/dng.c \
	coders/dng.h \
	coders/dot.c \
	coders/dot.h \
	coders/dps.h \
	coders/dpx.c \
	coders/dpx.h \
	coders/emf.h \
	coders/ept.h \
	coders/exr.h \
	coders/farbfeld.c \
	coders/farbfeld.h \
	coders/fax.c \
	coders/fax.h \
	coders/fits.c \
	coders/fits.h \
	coders/fl32.c \
	coders/fl32.h \
	coders/flif.h \
	coders/fpx.h \
	coders/ftxt.h \
	coders/ftxt.c \
	coders/ghostscript-private.h \
	coders/gif.c \
	coders/gif.h \
	coders/gradient.c \
	coders/gradient.h \
	coders/gray.c \
	coders/gray.h \
	coders/hald.c \
	coders/hald.h \
	coders/hdr.c \
	coders/hdr.h \
	coders/heic.h \
	coders/histogram.c \
	coders/histogram.h \
	coders/hrz.c \
	coders/hrz.h \
	coders/html.c \
	coders/html.h \
	coders/icon.c \
	coders/icon.h \
	coders/info.c \
	coders/info.h \
	coders/inline.c \
	coders/inline.h \
	coders/ipl.c \
	coders/ipl.h \
	coders/jbig.h \
	coders/jnx.c \
	coders/jnx.h \
	coders/jp2.h \
	coders/jpeg.h \
	coders/json.c \
	coders/json.h \
	coders/jxl.h \
	coders/kernel.c \
	coders/kernel.h \
	coders/label.c \
	coders/label.h \
	coders/mac.c \
	coders/mac.h \
	coders/magick.c \
	coders/magick.h \
	coders/map.c \
	coders/map.h \
	coders/mask.c \
	coders/mask.h \
	coders/mat.c \
	coders/mat.h \
	coders/matte.c \
	coders/matte.h \
	coders/meta.c \
	coders/meta.h \
	coders/miff.c \
	coders/miff.h \
	coders/mono.c \
	coders/mono.h \
	coders/mpc.c \
	coders/mpc.h \
	coders/mpr.c \
	coders/mpr.h \
	coders/msl.c \
	coders/msl.h \
	coders/mtv.c \
	coders/mtv.h \
	coders/mvg.c \
	coders/mvg.h \
	coders/null.c \
	coders/null.h \
	coders/ora.c \
	coders/ora.h \
	coders/otb.c \
	coders/otb.h \
	coders/palm.c \
	coders/palm.h \
	coders/pango.c \
	coders/pango.h \
	coders/pattern.c \
	coders/pattern.h \
	coders/pcd.c \
	coders/pcd.h \
	coders/pcl.c \
	coders/pcl.h \
	coders/pcx.c \
	coders/pcx.h \
	coders/pdb.c \
	coders/pdb.h \
	coders/pdf.c \
	coders/pdf.h \
	coders/pes.c \
	coders/pes.h \
	coders/pgx.c \
	coders/pgx.h \
	coders/pict.c \
	coders/pict.h \
	coders/pix.c \
	coders/pix.h \
	coders/plasma.c \
	coders/plasma.h \
	coders/png.h \
	coders/pnm.c \
	coders/pnm.h \
	coders/ps2.c \
	coders/ps2.h \
	coders/ps3.c \
	coders/ps3.h \
	coders/ps.c \
	coders/psd.c \
	coders/psd.h \
	coders/psd-private.h \
	coders/psd-private.h \
	coders/ps.h \
	coders/pwp.c \
	coders/pwp.h \
	coders/qoi.c \
	coders/qoi.h \
	coders/raw.c \
	coders/raw.h \
	coders/rgb.c \
	coders/rgb.h \
	coders/rgf.c \
	coders/rgf.h \
	coders/rla.c \
	coders/rla.h \
	coders/rle.c \
	coders/rle.h \
	coders/scr.c \
	coders/screenshot.c \
	coders/screenshot.h \
	coders/scr.h \
	coders/sct.c \
	coders/sct.h \
	coders/sf3.c \
	coders/sf3.h \
	coders/sfw.c \
	coders/sfw.h \
	coders/sgi.c \
	coders/sgi.h \
	coders/sixel.c \
	coders/sixel.h \
	coders/stegano.c \
	coders/stegano.h \
	coders/strimg.c \
	coders/strimg.h \
	coders/sun.c \
	coders/sun.h \
	coders/svg.c \
	coders/svg.h \
	coders/tga.c \
	coders/tga.h \
	coders/thumbnail.c \
	coders/thumbnail.h \
	coders/tiff.h \
	coders/tile.c \
	coders/tile.h \
	coders/tim2.c \
	coders/tim2.h \
	coders/tim.c \
	coders/tim.h \
	coders/ttf.c \
	coders/ttf.h \
	coders/txt.c \
	coders/txt.h \
	coders/uhdr.h \
	coders/uil.c \
	coders/uil.h \
	coders/url.c \
	coders/url.h \
	coders/uyvy.c \
	coders/uyvy.h \
	coders/vicar.c \
	coders/vicar.h \
	coders/vid.c \
	coders/vid.h \
	coders/video.c \
	coders/video.h \
	coders/viff.c \
	coders/viff.h \
	coders/vips.c \
	coders/vips.h \
	coders/wbmp.c \
	coders/wbmp.h \
	coders/webp.h \
	coders/wmf.h \
	coders/wpg.c \
	coders/wpg.h \
	coders/xbm.c \
	coders/xbm.h \
	coders/xc.c \
	coders/xcf.c \
	coders/xcf.h \
	coders/xc.h \
	coders/x.h \
	coders/xpm.c \
	coders/xpm.h \
	coders/xps.c \
	coders/xps.h \
	coders/xwd.h \
	coders/yaml.c \
	coders/yaml.h \
	coders/ycbcr.c \
	coders/ycbcr.h \
	coders/yuv.c \
	coders/yuv.h \
	$(MAGICKCORE_DPS_SRCS) \
	$(MAGICKCORE_DJVU_SRCS) \
	$(MAGICKCORE_DMR_SRCS) \
	$(MAGICKCORE_EXR_SRCS) \
	$(MAGICKCORE_FLIF_SRCS) \
	$(MAGICKCORE_FPX_SRCS) \
	$(MAGICKCORE_GDI32_SRCS) \
	$(MAGICKCORE_HEIC_SRCS) \
	$(MAGICKCORE_JBIG_SRCS) \
	$(MAGICKCORE_JPEG_SRCS) \
	$(MAGICKCORE_JP2_SRCS) \
	$(MAGICKCORE_JXL_SRCS) \
	$(MAGICKCORE_PNG_SRCS) \
	$(MAGICKCORE_TIFF_SRCS) \
	$(MAGICKCORE_UHDR_SRCS) \
	$(MAGICKCORE_WEBP_SRCS) \
	$(MAGICKCORE_WMF_SRCS) \
	$(MAGICKCORE_X11_SRCS)

CODERS_NOINST_HDRS = \
	coders/aai.h \
	coders/art.h \
	coders/ashlar.h \
	coders/avs.h \
	coders/bayer.h \
	coders/bgr.h \
	coders/bmp.h \
	coders/braille.h \
	coders/bytebuffer-private.h \
	coders/cals.h \
	coders/caption.h \
	coders/cin.h \
	coders/cip.h \
	coders/clipboard.h \
	coders/clip.h \
	coders/cmyk.h \
	coders/coders.h \
	coders/coders-list.h \
	coders/coders-private.h \
	coders/cube.h \
	coders/cut.h \
	coders/dcm.h \
	coders/dds.h \
	coders/debug.h \
	coders/dib.h \
	coders/djvu.h \
	coders/dng.h \
	coders/dot.h \
	coders/dps.h \
	coders/dpx.h \
	coders/emf.h \
	coders/ept.h \
	coders/exr.h \
	coders/farbfeld.h \
	coders/fax.h \
	coders/fits.h \
	coders/fl32.h \
	coders/flif.h \
	coders/fpx.h \
	coders/ftxt.h \
	coders/ghostscript-private.h \
	coders/gif.h \
	coders/gradient.h \
	coders/gray.h \
	coders/hald.h \
	coders/hdr.h \
	coders/heic.h \
	coders/histogram.h \
	coders/hrz.h \
	coders/html.h \
	coders/icon.h \
	coders/info.h \
	coders/inline.h \
	coders/ipl.h \
	coders/jbig.h \
	coders/jnx.h \
	coders/jp2.h \
	coders/jpeg.h \
	coders/json.h \
	coders/jxl.h \
	coders/kernel.h \
	coders/label.h \
	coders/mac.h \
	coders/magick.h \
	coders/map.h \
	coders/mask.h \
	coders/mat.h \
	coders/matte.h \
	coders/meta.h \
	coders/miff.h \
	coders/mono.h \
	coders/mpc.h \
	coders/mpr.h \
	coders/msl.h \
	coders/mtv.h \
	coders/mvg.h \
	coders/null.h \
	coders/ora.h \
	coders/otb.h \
	coders/palm.h \
	coders/pango.h \
	coders/pattern.h \
	coders/pcd.h \
	coders/pcl.h \
	coders/pcx.h \
	coders/pdb.h \
	coders/pdf.h \
	coders/pes.h \
	coders/pgx.h \
	coders/pict.h \
	coders/pix.h \
	coders/plasma.h \
	coders/png.h \
	coders/pnm.h \
	coders/ps2.h \
	coders/ps3.h \
	coders/psd.h \
	coders/psd-private.h \
	coders/ps.h \
	coders/pwp.h \
	coders/qoi.h \
	coders/raw.h \
	coders/rgb.h \
	coders/rgf.h \
	coders/rla.h \
	coders/rle.h \
	coders/screenshot.h \
	coders/scr.h \
	coders/sct.h \
	coders/sf3.h \
	coders/sfw.h \
	coders/sgi.h \
	coders/sixel.h \
	coders/stegano.h \
	coders/strimg.h \
	coders/sun.h \
	coders/svg.h \
	coders/tga.h \
	coders/thumbnail.h \
	coders/tiff.h \
	coders/tile.h \
	coders/tim.h \
	coders/tim2.h \
	coders/ttf.h \
	coders/txt.h \
	coders/uhdr.h \
	coders/uil.h \
	coders/url.h \
	coders/uyvy.h \
	coders/vicar.h \
	coders/vid.h \
	coders/video.h \
	coders/viff.h \
	coders/vips.h \
	coders/wbmp.h \
	coders/webp.h \
	coders/wmf.h \
	coders/wpg.h \
	coders/xbm.h \
	coders/xcf.h \
	coders/xc.h \
	coders/x.h \
	coders/xpm.h \
	coders/xps.h \
	coders/xwd.h \
	coders/yaml.h \
	coders/ycbcr.h \
	coders/yuv.h

if WITH_MODULES
coders_LTLIBRARIES = \
	coders/aai.la \
	coders/art.la \
	coders/ashlar.la \
	coders/avs.la \
	coders/bayer.la \
	coders/bgr.la \
	coders/bmp.la \
	coders/braille.la \
	coders/cals.la \
	coders/caption.la \
	coders/cin.la \
	coders/cip.la \
	coders/clip.la \
	coders/cmyk.la \
	coders/cube.la \
	coders/cut.la \
	coders/dcm.la \
	coders/dds.la \
	coders/debug.la \
	coders/dib.la \
	coders/dng.la \
	coders/dot.la \
	coders/dpx.la \
	coders/farbfeld.la \
	coders/fax.la \
	coders/fits.la \
	coders/fl32.la \
	coders/ftxt.la \
	coders/gif.la \
	coders/gradient.la \
	coders/gray.la \
	coders/hald.la \
	coders/hdr.la \
	coders/histogram.la \
	coders/hrz.la \
	coders/html.la \
	coders/icon.la \
	coders/info.la \
	coders/inline.la \
	coders/ipl.la \
	coders/jnx.la \
	coders/json.la \
	coders/kernel.la \
	coders/label.la \
	coders/mac.la \
	coders/magick.la \
	coders/map.la \
	coders/mask.la \
	coders/mat.la \
	coders/matte.la \
	coders/meta.la \
	coders/miff.la \
	coders/mono.la \
	coders/mpc.la \
	coders/mpr.la \
	coders/msl.la \
	coders/mtv.la \
	coders/mvg.la \
	coders/null.la \
	coders/ora.la \
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
	coders/pgx.la \
	coders/pict.la \
	coders/pix.la \
	coders/plasma.la \
	coders/pnm.la \
	coders/ps.la \
	coders/ps2.la \
	coders/ps3.la \
	coders/psd.la \
	coders/pwp.la \
	coders/qoi.la \
	coders/raw.la \
	coders/rgb.la \
	coders/rgf.la \
	coders/rla.la \
	coders/rle.la \
	coders/scr.la \
	coders/sct.la \
	coders/sf3.la \
	coders/sfw.la \
	coders/sgi.la \
	coders/sixel.la \
	coders/stegano.la \
	coders/strimg.la \
	coders/sun.la \
	coders/svg.la \
	coders/tga.la \
	coders/thumbnail.la \
	coders/tile.la \
	coders/tim.la \
	coders/tim2.la \
	coders/ttf.la \
	coders/txt.la \
	coders/uil.la \
	coders/url.la \
	coders/uyvy.la \
	coders/vicar.la \
	coders/vid.la \
	coders/video.la \
	coders/viff.la \
	coders/vips.la \
	coders/wbmp.la \
	coders/wpg.la \
	coders/xbm.la \
	coders/xc.la \
	coders/xcf.la \
	coders/xpm.la \
	coders/xps.la \
	coders/yaml.la \
	coders/ycbcr.la \
	coders/yuv.la \
	$(MAGICKCORE_DPS_MODULES) \
	$(MAGICKCORE_DJVU_MODULES) \
	$(MAGICKCORE_DMR_MODULES) \
	$(MAGICKCORE_EXR_MODULES) \
	$(MAGICKCORE_FLIF_MODULES) \
	$(MAGICKCORE_FPX_MODULES) \
	$(MAGICKCORE_GDI32_MODULES)  \
	$(MAGICKCORE_HEIC_MODULES)  \
	$(MAGICKCORE_JBIG_MODULES) \
	$(MAGICKCORE_JPEG_MODULES) \
	$(MAGICKCORE_JP2_MODULES) \
	$(MAGICKCORE_JXL_MODULES) \
	$(MAGICKCORE_PNG_MODULES) \
	$(MAGICKCORE_TIFF_MODULES) \
	$(MAGICKCORE_UHDR_MODULES) \
	$(MAGICKCORE_WEBP_MODULES) \
	$(MAGICKCORE_WMF_MODULES) \
	$(MAGICKCORE_X11_MODULES)
else
coders_LTLIBRARIES =
endif # WITH_MODULES

# AAI coder module
coders_aai_la_SOURCES      = coders/aai.c
coders_aai_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_aai_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_aai_la_LIBADD       = $(MAGICKCORE_LIBS)

# ART coder module
coders_art_la_SOURCES      = coders/art.c
coders_art_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_art_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_art_la_LIBADD       = $(MAGICKCORE_LIBS)

# ASHLAR coder module
coders_ashlar_la_SOURCES   = coders/ashlar.c
coders_ashlar_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_ashlar_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_ashlar_la_LIBADD    = $(MAGICKCORE_LIBS)

# AVS coder module
coders_avs_la_SOURCES      = coders/avs.c
coders_avs_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_avs_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_avs_la_LIBADD       = $(MAGICKCORE_LIBS)

# BAYER coder module
coders_bayer_la_SOURCES     = coders/bayer.c
coders_bayer_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_bayer_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_bayer_la_LIBADD      = $(MAGICKCORE_LIBS)

# BRG coder module
coders_bgr_la_SOURCES      = coders/bgr.c
coders_bgr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_bgr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_bgr_la_LIBADD       = $(MAGICKCORE_LIBS)

# BMP coder module
coders_bmp_la_SOURCES      = coders/bmp.c
coders_bmp_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_bmp_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_bmp_la_LIBADD       = $(MAGICKCORE_LIBS)

# BRAILLE coder module
coders_braille_la_SOURCES  = coders/braille.c
coders_braille_la_CPPFLAGS = $(MAGICK_CODER_CPPFLAGS)
coders_braille_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_braille_la_LIBADD   = $(MAGICKCORE_LIBS)

# CALS coder module
coders_cals_la_SOURCES     = coders/cals.c
coders_cals_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_cals_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_cals_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# CAPTION coder module
coders_caption_la_SOURCES  = coders/caption.c
coders_caption_la_CPPFLAGS = $(MAGICK_CODER_CPPFLAGS)
coders_caption_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_caption_la_LIBADD   = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# CINEON coder module
coders_cin_la_SOURCES      = coders/cin.c
coders_cin_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_cin_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cin_la_LIBADD       = $(MAGICKCORE_LIBS)

# CIP coder module
coders_cip_la_SOURCES      = coders/cip.c
coders_cip_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_cip_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cip_la_LIBADD       = $(MAGICKCORE_LIBS)

# CLIP coder module
coders_clip_la_SOURCES     = coders/clip.c
coders_clip_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_clip_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_clip_la_LIBADD      = $(MAGICKCORE_LIBS)

# CLIPBOARD coder module
coders_clipboard_la_SOURCES= coders/clipboard.c
coders_clipboard_la_CPPFLAGS= $(MAGICK_CODER_CPPFLAGS)
coders_clipboard_la_LDFLAGS= $(MODULECOMMONFLAGS)
coders_clipboard_la_LIBADD = $(MAGICKCORE_LIBS) $(GDI32_LIBS)

# CMYK coder module
coders_cmyk_la_SOURCES     = coders/cmyk.c
coders_cmyk_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_cmyk_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_cmyk_la_LIBADD      = $(MAGICKCORE_LIBS)

# CUBE coder module
coders_cube_la_SOURCES     = coders/cube.c
coders_cube_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_cube_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_cube_la_LIBADD      = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# CUT coder module
coders_cut_la_SOURCES      = coders/cut.c
coders_cut_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_cut_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_cut_la_LIBADD       = $(MAGICKCORE_LIBS)

# DCM coder module
coders_dcm_la_SOURCES      = coders/dcm.c
coders_dcm_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dcm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dcm_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# DDS coder module
coders_dds_la_SOURCES      = coders/dds.c
coders_dds_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dds_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dds_la_LIBADD       = $(MAGICKCORE_LIBS)

# DEBUG coder module
coders_debug_la_SOURCES    = coders/debug.c
coders_debug_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_debug_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_debug_la_LIBADD     = $(MAGICKCORE_LIBS)

# DIB coder module
coders_dib_la_SOURCES      = coders/dib.c
coders_dib_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dib_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dib_la_LIBADD       = $(MAGICKCORE_LIBS)

# DJVU coder module
coders_djvu_la_SOURCES     = coders/djvu.c
coders_djvu_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_djvu_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_djvu_la_LIBADD      = $(MAGICKCORE_LIBS) $(DJVU_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# DMR coder module
coders_dmr_la_SOURCES      = coders/dmr.c
coders_dmr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dmr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dmr_la_LIBADD       = $(MAGICKCORE_LIBS) $(DMR_LIBS)

# DNG coder module
coders_dng_la_SOURCES      = coders/dng.c
coders_dng_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dng_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dng_la_LIBADD       = $(MAGICKCORE_LIBS) $(RAW_R_LIBS) $(GOMP_LIBS)

# DPS coder module
coders_dps_la_SOURCES      = coders/dps.c
coders_dps_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dps_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dps_la_LIBADD       = $(MAGICKCORE_LIBS) $(DPS_LIBS) $(XEXT_LIBS) $(X11_LIBS)

# DPX coder module
coders_dpx_la_SOURCES      = coders/dpx.c
coders_dpx_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_dpx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dpx_la_LIBADD       = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# DOT coder module
coders_dot_la_SOURCES      = coders/dot.c
coders_dot_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS) $(GVC_CFLAGS)
coders_dot_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_dot_la_LIBADD       = $(MAGICKCORE_LIBS) $(GVC_LIBS)

# EMF coder module
coders_emf_la_SOURCES      = coders/emf.c
coders_emf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_emf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_emf_la_LIBADD       = $(MAGICKCORE_LIBS) $(GDI32_LIBS)

# EPT coder module
coders_ept_la_SOURCES      = coders/ept.c
coders_ept_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ept_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ept_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# EXR coder module
coders_exr_la_SOURCES      = coders/exr.c
coders_exr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS) $(OPENEXR_CFLAGS)
coders_exr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_exr_la_LIBADD       = $(MAGICKCORE_LIBS) $(OPENEXR_LIBS) $(MATH_LIBS)

# FARBFELD coder module
coders_farbfeld_la_SOURCES     = coders/farbfeld.c
coders_farbfeld_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_farbfeld_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_farbfeld_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# FAX coder module
coders_fax_la_SOURCES      = coders/fax.c
coders_fax_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_fax_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_fax_la_LIBADD       = $(MAGICKCORE_LIBS)

# FITS coder module
coders_fits_la_SOURCES     = coders/fits.c
coders_fits_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_fits_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_fits_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# FL32 coder module
coders_fl32_la_SOURCES     = coders/fl32.c
coders_fl32_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_fl32_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_fl32_la_LIBADD      = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# FLIF coder module
coders_flif_la_SOURCES      = coders/flif.c
coders_flif_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_flif_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_flif_la_LIBADD       = $(MAGICKCORE_LIBS) $(FLIF_LIBS)

# FPX coder module
coders_fpx_la_SOURCES      = coders/fpx.c
coders_fpx_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_fpx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_fpx_la_LIBADD       = $(MAGICKCORE_LIBS) $(FPX_LIBS) $(MATH_LIBS) -lstdc++

# FTXT coder module
coders_ftxt_la_SOURCES      = coders/ftxt.c
coders_ftxt_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ftxt_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ftxt_la_LIBADD       = $(MAGICKCORE_LIBS)

# GIF coder module
coders_gif_la_SOURCES      = coders/gif.c
coders_gif_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_gif_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_gif_la_LIBADD       = $(MAGICKCORE_LIBS)

# GRAY coder module
coders_gray_la_SOURCES     = coders/gray.c
coders_gray_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_gray_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_gray_la_LIBADD      = $(MAGICKCORE_LIBS)

# GRADIENT coder module
coders_gradient_la_SOURCES = coders/gradient.c
coders_gradient_la_CPPFLAGS= $(MAGICK_CODER_CPPFLAGS)
coders_gradient_la_LDFLAGS = $(MODULECOMMONFLAGS)
coders_gradient_la_LIBADD  = $(MAGICKCORE_LIBS)

# HISTOGRAM coder module
coders_histogram_la_SOURCES = coders/histogram.c
coders_histogram_la_CPPFLAGS= $(MAGICK_CODER_CPPFLAGS)
coders_histogram_la_LDFLAGS = $(MODULECOMMONFLAGS)
coders_histogram_la_LIBADD  = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# HALD coder module
coders_hald_la_SOURCES     = coders/hald.c
coders_hald_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_hald_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_hald_la_LIBADD      = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# HDR coder module
coders_hdr_la_SOURCES      = coders/hdr.c
coders_hdr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_hdr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_hdr_la_LIBADD       = $(MAGICKCORE_LIBS) $(GOMP_LIBS) $(MATH_LIBS)

# HEIC coder module
coders_heic_la_SOURCES     = coders/heic.c
coders_heic_la_CPPFLAGS    = $(MODULE_CODER_CPPFLAGS)
coders_heic_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_heic_la_LIBADD      = $(MAGICKCORE_LIBS) $(HEIF_LIBS)

# HRZ coder module
coders_hrz_la_SOURCES      = coders/hrz.c
coders_hrz_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_hrz_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_hrz_la_LIBADD       = $(MAGICKCORE_LIBS)

# HTML coder module
coders_html_la_SOURCES     = coders/html.c
coders_html_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_html_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_html_la_LIBADD      = $(MAGICKCORE_LIBS)

# ICON coder module
coders_icon_la_SOURCES     = coders/icon.c
coders_icon_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_icon_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_icon_la_LIBADD      = $(MAGICKCORE_LIBS)

# INFO coder module
coders_info_la_SOURCES     = coders/info.c
coders_info_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_info_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_info_la_LIBADD      = $(MAGICKCORE_LIBS)

# INLINE coder module
coders_inline_la_SOURCES   = coders/inline.c
coders_inline_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_inline_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_inline_la_LIBADD    = $(MAGICKCORE_LIBS)

# IPL coder module
coders_ipl_la_SOURCES      = coders/ipl.c
coders_ipl_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ipl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ipl_la_LIBADD       = $(MAGICKCORE_LIBS)

# JBIG coder module
coders_jbig_la_SOURCES     = coders/jbig.c
coders_jbig_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_jbig_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_jbig_la_LIBADD      = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(MATH_LIBS)

# JPEG coder module
coders_jpeg_la_SOURCES     = coders/jpeg.c
coders_jpeg_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_jpeg_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_jpeg_la_LIBADD      = $(MAGICKCORE_LIBS) $(JPEG_LIBS) $(MATH_LIBS)

# JPEG 2000 coder module
coders_jp2_la_SOURCES      = coders/jp2.c
coders_jp2_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS) $(LIBOPENJP2_CFLAGS)
coders_jp2_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_jp2_la_LIBADD       = $(MAGICKCORE_LIBS) $(LIBOPENJP2_LIBS) $(MATH_LIBS)

# JNX coder module
coders_jnx_la_SOURCES      = coders/jnx.c
coders_jnx_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_jnx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_jnx_la_LIBADD       = $(MAGICKCORE_LIBS)

# JSON coder module
coders_json_la_SOURCES     = coders/json.c
coders_json_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_json_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_json_la_LIBADD      = $(MAGICKCORE_LIBS)

# JXL coder module
coders_jxl_la_SOURCES      = coders/jxl.c
coders_jxl_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS) $(JXL_CFLAGS)
coders_jxl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_jxl_la_LIBADD       = $(MAGICKCORE_LIBS) $(JXL_LIBS)

# KERNEL coder module
coders_kernel_la_SOURCES   = coders/kernel.c
coders_kernel_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_kernel_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_kernel_la_LIBADD    = $(MAGICKCORE_LIBS)

# LABEL coder module
coders_label_la_SOURCES    = coders/label.c
coders_label_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_label_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_label_la_LIBADD     = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# MAC coder module
coders_mac_la_SOURCES      = coders/mac.c
coders_mac_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mac_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mac_la_LIBADD       = $(MAGICKCORE_LIBS)

# MAGICK coder module
coders_magick_la_SOURCES   = coders/magick.c
coders_magick_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_magick_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_magick_la_LIBADD    = $(MAGICKCORE_LIBS)

# MAP coder module
coders_map_la_SOURCES      = coders/map.c
coders_map_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_map_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_map_la_LIBADD       = $(MAGICKCORE_LIBS)

# MASK coder module
coders_mask_la_SOURCES     = coders/mask.c
coders_mask_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_mask_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_mask_la_LIBADD      = $(MAGICKCORE_LIBS)

# MAT coder module
coders_mat_la_SOURCES      = coders/mat.c
coders_mat_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mat_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mat_la_LIBADD       = $(MAGICKCORE_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# MATTE coder module
coders_matte_la_SOURCES    = coders/matte.c
coders_matte_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_matte_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_matte_la_LIBADD     = $(MAGICKCORE_LIBS)

# META coder module
coders_meta_la_SOURCES     = coders/meta.c
coders_meta_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_meta_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_meta_la_LIBADD      = $(MAGICKCORE_LIBS)

# MIFF coder module
coders_miff_la_SOURCES     = coders/miff.c
coders_miff_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_miff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_miff_la_LIBADD      = $(MAGICKCORE_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(BZLIB_LIBS)

# MONO coder module
coders_mono_la_SOURCES     = coders/mono.c
coders_mono_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_mono_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_mono_la_LIBADD      = $(MAGICKCORE_LIBS)

# MPC coder module
coders_mpc_la_SOURCES      = coders/mpc.c
coders_mpc_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mpc_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mpc_la_LIBADD       = $(MAGICKCORE_LIBS)

# MPR coder module
coders_mpr_la_SOURCES      = coders/mpr.c
coders_mpr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mpr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mpr_la_LIBADD       = $(MAGICKCORE_LIBS)

# MSL coder module
coders_msl_la_SOURCES      = coders/msl.c
coders_msl_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_msl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_msl_la_LIBADD       = $(MAGICKCORE_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# MTV coder module
coders_mtv_la_SOURCES      = coders/mtv.c
coders_mtv_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mtv_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mtv_la_LIBADD       = $(MAGICKCORE_LIBS)

# MVG coder module
coders_mvg_la_SOURCES      = coders/mvg.c
coders_mvg_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_mvg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_mvg_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# NULL coder module
coders_null_la_SOURCES     = coders/null.c
coders_null_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_null_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_null_la_LIBADD      = $(MAGICKCORE_LIBS)

# ORA coder module
coders_ora_la_SOURCES     = coders/ora.c
coders_ora_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_ora_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_ora_la_LIBADD      = $(MAGICKCORE_LIBS) $(ZIP_LIBS)

# OTB coder module
coders_otb_la_SOURCES      = coders/otb.c
coders_otb_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_otb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_otb_la_LIBADD       = $(MAGICKCORE_LIBS)

# PALM coder module
coders_palm_la_SOURCES     = coders/palm.c
coders_palm_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_palm_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_palm_la_LIBADD      = $(MAGICKCORE_LIBS)

# PANGO coder module
coders_pango_la_SOURCES    = coders/pango.c
coders_pango_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS) $(PANGO_CFLAGS)
coders_pango_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_pango_la_LIBADD     = $(MAGICKCORE_LIBS) $(PANGO_LIBS)

# Pattern module
coders_pattern_la_SOURCES  = coders/pattern.c
coders_pattern_la_CPPFLAGS = $(MAGICK_CODER_CPPFLAGS)
coders_pattern_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_pattern_la_LIBADD   = $(MAGICKCORE_LIBS)

# PCD coder module
coders_pcd_la_SOURCES      = coders/pcd.c
coders_pcd_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pcd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcd_la_LIBADD       = $(MAGICKCORE_LIBS)

# PCL coder module
coders_pcl_la_SOURCES      = coders/pcl.c
coders_pcl_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pcl_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcl_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# PCX coder module
coders_pcx_la_SOURCES      = coders/pcx.c
coders_pcx_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pcx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pcx_la_LIBADD       = $(MAGICKCORE_LIBS)

# PDB coder module
coders_pdb_la_SOURCES      = coders/pdb.c
coders_pdb_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pdb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pdb_la_LIBADD       = $(MAGICKCORE_LIBS)

# PDF coder module
coders_pdf_la_SOURCES      = coders/pdf.c
coders_pdf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pdf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pdf_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(GS_LIBS) $(MATH_LIBS)

# PES coder module
coders_pes_la_SOURCES      = coders/pes.c
coders_pes_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pes_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pes_la_LIBADD       = $(MAGICKCORE_LIBS)

# PGX coder module
coders_pgx_la_SOURCES      = coders/pgx.c
coders_pgx_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pgx_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pgx_la_LIBADD       = $(MAGICKCORE_LIBS)

# PICT coder module
coders_pict_la_SOURCES     = coders/pict.c
coders_pict_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_pict_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_pict_la_LIBADD      = $(MAGICKCORE_LIBS)

# PIX coder module
coders_pix_la_SOURCES      = coders/pix.c
coders_pix_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pix_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pix_la_LIBADD       = $(MAGICKCORE_LIBS)

# PNG coder module
coders_png_la_SOURCES      = coders/png.c
coders_png_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_png_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_png_la_LIBADD       = $(MAGICKCORE_LIBS) $(PNG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# PLASMA coder module
coders_plasma_la_SOURCES   = coders/plasma.c
coders_plasma_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_plasma_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_plasma_la_LIBADD    = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# PNM coder module
coders_pnm_la_SOURCES      = coders/pnm.c
coders_pnm_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pnm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pnm_la_LIBADD       = $(MAGICKCORE_LIBS) $(GOMP_LIBS)

# PS coder module
coders_ps_la_SOURCES       = coders/ps.c
coders_ps_la_CPPFLAGS      = $(MAGICK_CODER_CPPFLAGS)
coders_ps_la_LDFLAGS       = $(MODULECOMMONFLAGS)
coders_ps_la_LIBADD        = $(MAGICKCORE_LIBS) $(GS_LIBS) $(MATH_LIBS)

# PS2 coder module
coders_ps2_la_SOURCES      = coders/ps2.c
coders_ps2_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ps2_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ps2_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(ZSTD_LIBS) $(WEBPMUX_LIBS) $(WEBP_LIBS) $(MATH_LIBS)

# PS3 coder module
coders_ps3_la_SOURCES      = coders/ps3.c
coders_ps3_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ps3_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ps3_la_LIBADD       = $(MAGICKCORE_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(ZSTD_LIBS) $(WEBPMUX_LIBS) $(WEBP_LIBS) $(MATH_LIBS)

# PSD coder module
coders_psd_la_SOURCES      = coders/psd.c coders/psd-private.h
coders_psd_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_psd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_psd_la_LIBADD       = $(MAGICKCORE_LIBS)

# PWP coder module
coders_pwp_la_SOURCES      = coders/pwp.c
coders_pwp_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_pwp_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_pwp_la_LIBADD       = $(MAGICKCORE_LIBS)

# QOI coder module
coders_qoi_la_SOURCES      = coders/qoi.c
coders_qoi_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_qoi_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_qoi_la_LIBADD       = $(MAGICKCORE_LIBS)

# RAW coder module
coders_raw_la_SOURCES      = coders/raw.c
coders_raw_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_raw_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_raw_la_LIBADD       = $(MAGICKCORE_LIBS)

# RGB coder module
coders_rgb_la_SOURCES      = coders/rgb.c
coders_rgb_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_rgb_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rgb_la_LIBADD       = $(MAGICKCORE_LIBS)

# RGF coder module
coders_rgf_la_SOURCES      = coders/rgf.c
coders_rgf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_rgf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rgf_la_LIBADD       = $(MAGICKCORE_LIBS)

# RLA coder module
coders_rla_la_SOURCES      = coders/rla.c
coders_rla_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_rla_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rla_la_LIBADD       = $(MAGICKCORE_LIBS)

# RLE coder module
coders_rle_la_SOURCES      = coders/rle.c
coders_rle_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_rle_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_rle_la_LIBADD       = $(MAGICKCORE_LIBS)

# SCT coder module
coders_sct_la_SOURCES      = coders/sct.c
coders_sct_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_sct_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sct_la_LIBADD       = $(MAGICKCORE_LIBS)

# SF3 coder module
coders_sf3_la_SOURCES      = coders/sf3.c
coders_sf3_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_sf3_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sf3_la_LIBADD       = $(MAGICKCORE_LIBS)

# SFW coder module
coders_sfw_la_SOURCES      = coders/sfw.c
coders_sfw_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_sfw_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sfw_la_LIBADD       = $(MAGICKCORE_LIBS)

# SGI coder module
coders_sgi_la_SOURCES      = coders/sgi.c
coders_sgi_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_sgi_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sgi_la_LIBADD       = $(MAGICKCORE_LIBS)

# SIXEL coder module
coders_sixel_la_SOURCES    = coders/sixel.c
coders_sixel_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_sixel_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_sixel_la_LIBADD     = $(MAGICKCORE_LIBS)

# STEGANO coder module
coders_stegano_la_SOURCES  = coders/stegano.c
coders_stegano_la_CPPFLAGS = $(MAGICK_CODER_CPPFLAGS)
coders_stegano_la_LDFLAGS  = $(MODULECOMMONFLAGS)
coders_stegano_la_LIBADD   = $(MAGICKCORE_LIBS)

# STRIMG coder module
coders_strimg_la_SOURCES   = coders/strimg.c
coders_strimg_la_CPPFLAGS  = $(MAGICK_CODER_CPPFLAGS)
coders_strimg_la_LDFLAGS   = $(MODULECOMMONFLAGS)
coders_strimg_la_LIBADD    = $(MAGICKCORE_LIBS)

# SUN coder module
coders_sun_la_SOURCES      = coders/sun.c
coders_sun_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_sun_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_sun_la_LIBADD       = $(MAGICKCORE_LIBS)

# SCR coder module
coders_scr_la_SOURCES      = coders/scr.c
coders_scr_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_scr_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_scr_la_LIBADD       = $(MAGICKCORE_LIBS)

# SVG coder module
coders_svg_la_SOURCES      = coders/svg.c
coders_svg_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS) $(RSVG_CFLAGS) $(CAIRO_SVG_CFLAGS)
coders_svg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_svg_la_LIBADD       = $(MAGICKCORE_LIBS) $(AUTOTRACE_LIBS) $(RSVG_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)

# TGA coder module
coders_tga_la_SOURCES      = coders/tga.c
coders_tga_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_tga_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_tga_la_LIBADD       = $(MAGICKCORE_LIBS)

# THUMBNAIL coder module
coders_thumbnail_la_SOURCES= coders/thumbnail.c
coders_thumbnail_la_CPPFLAGS= $(MAGICK_CODER_CPPFLAGS)
coders_thumbnail_la_LDFLAGS= $(MODULECOMMONFLAGS)
coders_thumbnail_la_LIBADD = $(MAGICKCORE_LIBS)

# TIFF coder module
coders_tiff_la_SOURCES     = coders/tiff.c coders/psd.c coders/psd-private.h
coders_tiff_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS) $(TIFF_CFLAGS)
coders_tiff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_tiff_la_LIBADD      = $(MAGICKCORE_LIBS) $(TIFF_LIBS) $(JBIG_LIBS) $(JPEG_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(ZSTD_LIBS) $(WEBPMUX_LIBS) $(WEBP_LIBS) $(MATH_LIBS)

# TILE coder module
coders_tile_la_SOURCES     = coders/tile.c
coders_tile_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_tile_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_tile_la_LIBADD      = $(MAGICKCORE_LIBS)

# TIM coder module
coders_tim_la_SOURCES      = coders/tim.c
coders_tim_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_tim_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_tim_la_LIBADD       = $(MAGICKCORE_LIBS)

# TIM2 coder module
coders_tim2_la_SOURCES      = coders/tim2.c
coders_tim2_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_tim2_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_tim2_la_LIBADD       = $(MAGICKCORE_LIBS)

# TTF coder module
coders_ttf_la_SOURCES      = coders/ttf.c
coders_ttf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_ttf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_ttf_la_LIBADD       = $(MAGICKCORE_LIBS)

# TXT coder module
coders_txt_la_SOURCES      = coders/txt.c
coders_txt_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_txt_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_txt_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)

# UHDR coder module
coders_uhdr_la_SOURCES     = coders/uhdr.c
coders_uhdr_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_uhdr_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_uhdr_la_LIBADD      = $(MAGICKCORE_LIBS) $(UHDR_LIBS) $(MATH_LIBS)

# UIL coder module
coders_uil_la_SOURCES      = coders/uil.c
coders_uil_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_uil_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_uil_la_LIBADD       = $(MAGICKCORE_LIBS)

# URL coder module
coders_url_la_SOURCES      = coders/url.c
coders_url_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_url_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_url_la_LIBADD       = $(MAGICKCORE_LIBS) $(XML_LIBS) $(LZMA_LIBS) $(ZLIB_LIBS) $(MATH_LIBS)
if WIN32_NATIVE_BUILD
coders_url_la_LIBADD += -lurlmon
endif

# UYVY coder module
coders_uyvy_la_SOURCES     = coders/uyvy.c
coders_uyvy_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_uyvy_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_uyvy_la_LIBADD      = $(MAGICKCORE_LIBS)

# VICAR coder module
coders_vicar_la_SOURCES    = coders/vicar.c
coders_vicar_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_vicar_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_vicar_la_LIBADD     = $(MAGICKCORE_LIBS)

# VID coder module
coders_vid_la_SOURCES      = coders/vid.c
coders_vid_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_vid_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_vid_la_LIBADD       = $(MAGICKCORE_LIBS)

# VIDEO coder module
coders_video_la_SOURCES     = coders/video.c
coders_video_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_video_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_video_la_LIBADD      = $(MAGICKCORE_LIBS)

# VIFF coder module
coders_viff_la_SOURCES     = coders/viff.c
coders_viff_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_viff_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_viff_la_LIBADD      = $(MAGICKCORE_LIBS)

# VIPS coder module
coders_vips_la_SOURCES     = coders/vips.c
coders_vips_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_vips_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_vips_la_LIBADD      = $(MAGICKCORE_LIBS)

# WBMP coder module
coders_wbmp_la_SOURCES     = coders/wbmp.c
coders_wbmp_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_wbmp_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_wbmp_la_LIBADD      = $(MAGICKCORE_LIBS)

# WEBP coder module
coders_webp_la_SOURCES     = coders/webp.c
coders_webp_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_webp_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_webp_la_LIBADD      = $(MAGICKCORE_LIBS) $(WEBPMUX_LIBS) $(WEBP_LIBS) $(MATH_LIBS)

# WMF coder module
coders_wmf_la_SOURCES      = coders/wmf.c
coders_wmf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_wmf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_wmf_la_LIBADD       = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(WMF_LIBS) $(MATH_LIBS)

# WPG coder module
coders_wpg_la_SOURCES      = coders/wpg.c
coders_wpg_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_wpg_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_wpg_la_LIBADD       = $(MAGICKCORE_LIBS)

# X coder module
coders_x_la_SOURCES        = coders/x.c
coders_x_la_CPPFLAGS       = $(MAGICK_CODER_CPPFLAGS)
coders_x_la_LDFLAGS        = $(MODULECOMMONFLAGS)
coders_x_la_LIBADD         = $(MAGICKCORE_LIBS) $(X11_LIBS)

# XBM coder module
coders_xbm_la_SOURCES      = coders/xbm.c
coders_xbm_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_xbm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xbm_la_LIBADD       = $(MAGICKCORE_LIBS)

# XC coder module
coders_xc_la_SOURCES       = coders/xc.c
coders_xc_la_CPPFLAGS      = $(MAGICK_CODER_CPPFLAGS)
coders_xc_la_LDFLAGS       = $(MODULECOMMONFLAGS)
coders_xc_la_LIBADD        = $(MAGICKCORE_LIBS)

# XCF coder module
coders_xcf_la_SOURCES      = coders/xcf.c
coders_xcf_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_xcf_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xcf_la_LIBADD       = $(MAGICKCORE_LIBS)

# XPM coder module
coders_xpm_la_SOURCES      = coders/xpm.c
coders_xpm_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_xpm_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xpm_la_LIBADD       = $(MAGICKCORE_LIBS)

# XPS coder module
coders_xps_la_SOURCES      = coders/xps.c
coders_xps_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_xps_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xps_la_LIBADD       = $(MAGICKCORE_LIBS) $(MUPDF_LIBS) $(FREETYPE_LIBS) $(MATH_LIBS)

# XWD coder module
coders_xwd_la_SOURCES      = coders/xwd.c
coders_xwd_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_xwd_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_xwd_la_LIBADD       = $(MAGICKCORE_LIBS) $(X11_LIBS)

# YAML coder module
coders_yaml_la_SOURCES     = coders/yaml.c
coders_yaml_la_CPPFLAGS    = $(MAGICK_CODER_CPPFLAGS)
coders_yaml_la_LDFLAGS     = $(MODULECOMMONFLAGS)
coders_yaml_la_LIBADD      = $(MAGICKCORE_LIBS)

# YCBCR coder module
coders_ycbcr_la_SOURCES    = coders/ycbcr.c
coders_ycbcr_la_CPPFLAGS   = $(MAGICK_CODER_CPPFLAGS)
coders_ycbcr_la_LDFLAGS    = $(MODULECOMMONFLAGS)
coders_ycbcr_la_LIBADD     = $(MAGICKCORE_LIBS)

# YUV coder module
coders_yuv_la_SOURCES      = coders/yuv.c
coders_yuv_la_CPPFLAGS     = $(MAGICK_CODER_CPPFLAGS)
coders_yuv_la_LDFLAGS      = $(MODULECOMMONFLAGS)
coders_yuv_la_LIBADD       = $(MAGICKCORE_LIBS)

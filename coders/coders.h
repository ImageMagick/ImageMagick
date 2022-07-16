/*
  Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization
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

/*
  Include declarations.
*/
#include "coders/aai.h"
#include "coders/art.h"
#include "coders/ashlar.h"
#include "coders/avs.h"
#include "coders/bayer.h"
#include "coders/bgr.h"
#include "coders/bmp.h"
#include "coders/braille.h"
#include "coders/cals.h"
#include "coders/caption.h"
#include "coders/cin.h"
#include "coders/cip.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  #include "coders/clipboard.h"
#endif
#include "coders/clip.h"
#include "coders/cmyk.h"
#include "coders/cube.h"
#include "coders/cut.h"
#include "coders/dcm.h"
#include "coders/dds.h"
#include "coders/debug.h"
#include "coders/dib.h"
#if defined(MAGICKCORE_DJVU_DELEGATE)
  #include "coders/djvu.h"
#endif
#include "coders/dng.h"
#if defined(MAGICKCORE_GVC_DELEGATE)
#include "coders/dot.h"
#endif
#if defined(MAGICKCORE_DPS_DELEGATE)
  #include "coders/dps.h"
#endif
#include "coders/dpx.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  #include "coders/emf.h"
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  #include "coders/ept.h"
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  #include "coders/exr.h"
#endif
#include "coders/farbfeld.h"
#include "coders/fax.h"
#include "coders/fits.h"
#include "coders/fl32.h"
#if defined(MAGICKCORE_FLIF_DELEGATE)
  #include "coders/flif.h"
#endif
#include "coders/fpx.h"
#include "coders/ftxt.h"
#include "coders/gif.h"
#include "coders/gradient.h"
#include "coders/gray.h"
#include "coders/hald.h"
#include "coders/hdr.h"
#if defined(MAGICKCORE_HEIC_DELEGATE)
  #include "coders/heic.h"
#endif
#include "coders/histogram.h"
#include "coders/hrz.h"
#include "coders/html.h"
#include "coders/icon.h"
#include "coders/info.h"
#include "coders/inline.h"
#include "coders/ipl.h"
#if defined(MAGICKCORE_JBIG_DELEGATE)
  #include "coders/jbig.h"
#endif
#include "coders/jnx.h"
#if defined(MAGICKCORE_JP2_DELEGATE) || defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  #include "coders/jp2.h"
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE)
  #include "coders/jpeg.h"
#endif
#include "coders/json.h"
#if defined(MAGICKCORE_JXL_DELEGATE)
  #include "coders/jxl.h"
#endif
#include "coders/kernel.h"
#include "coders/label.h"
#include "coders/mac.h"
#include "coders/magick.h"
#include "coders/map.h"
#include "coders/mask.h"
#include "coders/mat.h"
#include "coders/matte.h"
#include "coders/meta.h"
#include "coders/miff.h"
#include "coders/mono.h"
#include "coders/mpc.h"
#include "coders/mpr.h"
#include "coders/msl.h"
#include "coders/mtv.h"
#include "coders/mvg.h"
#include "coders/null.h"
#include "coders/ora.h"
#include "coders/otb.h"
#include "coders/palm.h"
#include "coders/pango.h"
#include "coders/pattern.h"
#include "coders/pcd.h"
#include "coders/pcl.h"
#include "coders/pcx.h"
#include "coders/pdb.h"
#include "coders/pdf.h"
#include "coders/pes.h"
#include "coders/pgx.h"
#include "coders/pict.h"
#include "coders/pix.h"
#include "coders/plasma.h"
#if defined(MAGICKCORE_PNG_DELEGATE)
  #include "coders/png.h"
#endif
#include "coders/pnm.h"
#include "coders/ps2.h"
#include "coders/ps3.h"
#include "coders/ps.h"
#include "coders/psd.h"
#include "coders/pwp.h"
#include "coders/qoi.h"
#include "coders/raw.h"
#include "coders/rgb.h"
#include "coders/rgf.h"
#include "coders/rla.h"
#include "coders/rle.h"
#include "coders/scr.h"
#include "coders/screenshot.h"
#include "coders/sct.h"
#include "coders/sfw.h"
#include "coders/sgi.h"
#include "coders/sixel.h"
#include "coders/stegano.h"
#include "coders/strimg.h"
#include "coders/sun.h"
#include "coders/svg.h"
#include "coders/tga.h"
#include "coders/thumbnail.h"
#if defined(MAGICKCORE_TIFF_DELEGATE)
  #include "coders/tiff.h"
#endif
#include "coders/tile.h"
#include "coders/tim.h"
#include "coders/tim2.h"
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  #include "coders/ttf.h"
#endif
#include "coders/txt.h"
#include "coders/uil.h"
#include "coders/url.h"
#include "coders/uyvy.h"
#include "coders/vicar.h"
#include "coders/vid.h"
#include "coders/video.h"
#include "coders/viff.h"
#include "coders/vips.h"
#include "coders/wbmp.h"
#if defined(MAGICKCORE_WEBP_DELEGATE)
  #include "coders/webp.h"
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  #include "coders/wmf.h"
#endif
#include "coders/wpg.h"
#include "coders/xbm.h"
#if defined(MAGICKCORE_X11_DELEGATE)
  #include "coders/x.h"
#endif
#include "coders/xc.h"
#include "coders/xcf.h"
#include "coders/xpm.h"
#include "coders/xps.h"
#if defined(MAGICKCORE_X11_DELEGATE)
  #include "coders/xwd.h"
#endif
#include "coders/yaml.h"
#include "coders/ycbcr.h"
#include "coders/yuv.h"

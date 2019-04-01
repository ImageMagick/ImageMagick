PROJECT(Coders C)

SET(CODERS_BASE_SRCS
#headers
aai.h 
art.h 
avs.h 
bgr.h 
bmp.h 
braille.h 
cals.h 
caption.h 
cin.h 
cip.h 
clipboard.h 
clip.h 
cmyk.h 
coders.h 
coders-list.h 
coders-private.h 
cube.h 
cut.h 
dcm.h 
dds.h 
debug.h 
dib.h 
djvu.h 
dng.h 
dot.h 
dps.h 
dpx.h 
emf.h 
ept.h 
exr.h 
fax.h 
fits.h 
flif.h 
fpx.h 
gif.h 
gradient.h 
gray.h 
hald.h 
hdr.h 
heic.h 
histogram.h 
hrz.h 
html.h 
icon.h 
info.h 
inline.h 
ipl.h 
jbig.h 
jnx.h 
jp2.h 
jpeg.h 
json.h 
label.h 
mac.h 
magick.h 
map.h 
mask.h 
mat.h 
matte.h 
meta.h 
miff.h 
mono.h 
mpc.h 
mpeg.h 
mpr.h 
msl.h 
mtv.h 
mvg.h 
null.h 
otb.h 
palm.h 
pango.h 
pattern.h 
pcd.h 
pcl.h 
pcx.h 
pdb.h 
pdf.h 
pes.h 
pgx.h 
pict.h 
pix.h 
plasma.h 
png.h 
pnm.h 
ps2.h 
ps3.h 
psd.h 
psd-private.h 
ps.h 
pwp.h 
raw.h 
rgb.h 
rgf.h 
rla.h 
rle.h 
screenshot.h 
scr.h 
sct.h 
sfw.h 
sgi.h 
sixel.h 
stegano.h 
sun.h 
svg.h 
tga.h 
thumbnail.h 
tiff.h 
tile.h 
tim.h 
ttf.h 
txt.h 
uil.h 
url.h 
uyvy.h 
vicar.h 
vid.h 
viff.h 
vips.h 
wbmp.h 
webp.h 
wmf.h 
wpg.h 
xbm.h 
xcf.h 
xc.h 
x.h 
xpm.h 
xps.h 
xtrn.h 
xwd.h 
ycbcr.h 
yuv.h 


#sources
aai.c 
art.c 
avs.c 
bgr.c 
bmp.c 
braille.c 
cals.c 
caption.c 
cin.c 
cip.c 
clip.c 
cmyk.c 
cube.c 
cut.c 
dcm.c 
dds.c 
debug.c 
dib.c 
dng.c 
dot.c 
dpx.c 
ept.c 
fax.c 
fits.c 
gif.c 
gradient.c 
gray.c 
hald.c 
hdr.c 
histogram.c 
hrz.c 
html.c 
icon.c 
info.c 
inline.c 
ipl.c 
jnx.c 
json.c 
label.c 
mac.c 
magick.c 
map.c 
mask.c 
mat.c 
matte.c 
meta.c 
miff.c 
mono.c 
mpc.c 
mpeg.c 
mpr.c 
msl.c 
mtv.c 
mvg.c 
null.c 
otb.c 
palm.c 
pango.c 
pattern.c 
pcd.c 
pcl.c 
pcx.c 
pdb.c 
pdf.c 
pes.c 
pgx.c 
pict.c 
pix.c 
plasma.c 
pnm.c 
ps.c 
ps2.c 
ps3.c 
psd.c 
psd-private.h 
pwp.c 
raw.c 
rgb.c 
rgf.c 
rla.c 
rle.c 
scr.c 
screenshot.c 
sct.c 
sfw.c 
sgi.c 
sixel.c 
stegano.c 
sun.c 
svg.c 
tga.c 
thumbnail.c 
tile.c 
tim.c 
ttf.c 
txt.c 
uil.c 
url.c 
uyvy.c 
vicar.c 
vid.c 
viff.c 
vips.c 
wbmp.c 
wpg.c 
xbm.c 
xc.c 
xcf.c 
xpm.c 
xps.c 
xtrn.c 
ycbcr.c 
yuv.c
)

# Let's fill a variable with the code for the available delegates
SET(CODERS_DELEGATES_SRCS "")
IF(DPS_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS dps.c)
ENDIF()
IF(DJVU_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS djvu.c)
ENDIF()
IF(OPENEXR_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS exr.c)
ENDIF()
IF(FLIF_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS flif.c)
ENDIF()
IF(FPX_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS fpx.c)
ENDIF()
IF(HEIC_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS heic.c)
ENDIF()
IF(JBIG_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS jbig.c)
ENDIF()
IF(JPEG_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS jpeg.c)
ENDIF()
IF(LIBOPENJP2_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS jp2.c)
ENDIF()
IF(PNG_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS png.c)
ENDIF()
IF(TIFF_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS tiff.c)
ENDIF()
IF(WEBP_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS webp.c)
ENDIF()
IF(WINGDI32_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS clipboard.c emf.c)
  # Seems that emf.c uses namespaces so we need to compile as c++
  SET_SOURCE_FILES_PROPERTIES(emf.c PROPERTIES LANGUAGE CXX)
ENDIF()
IF(WMF_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS wmf.c)
ENDIF()
IF(X11_DELEGATE)
  list(APPEND CODERS_DELEGATES_SRCS x.c xwd.c)
ENDIF()

ADD_LIBRARY(coders STATIC ${CODERS_BASE_SRCS} ${CODERS_DELEGATES_SRCS})
TARGET_INCLUDE_DIRECTORIES(coders PUBLIC ${IMAGEMAGICK_BASE_PATH})
TARGET_INCLUDE_DIRECTORIES(coders PRIVATE  ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
# Set the MAGICKCORE_QUANTUM_DEPTH to 16, this also avoids build being unsuccessful on MSVC by using of preprocessor #warning
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
# Seems we need to define _LIB if we want to build statically
# Set LZMA_API_STATIC because we are using a static lzma library
TARGET_COMPILE_DEFINITIONS(coders PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE -D_LIB -DLZMA_API_STATIC)

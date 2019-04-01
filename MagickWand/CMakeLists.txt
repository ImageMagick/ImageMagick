PROJECT(MagickWand C)

SET(MAGICKWAND_BASE_SRCS
#headers
MagickWand.h 
animate.h 
compare.h 
composite.h 
conjure.h 
convert.h 
deprecate.h 
display.h 
drawing-wand.h 
identify.h 
import.h 
magick-cli.h 
magick-image.h 
magick-property.h 
magick-wand-private.h
method-attribute.h 
mogrify.h 
mogrify-private.h
montage.h 
operation.h 
operation-private.h
pixel-iterator.h 
pixel-wand.h 
pixel-wand-private.h
script-token.h
stream.h 
studio.h
wand.h
wandcli.h 
wandcli-private.h
wand-view.h

#sources
animate.c 
compare.c
composite.c
conjure.c
convert.c
deprecate.c
display.c
drawing-wand.c
identify.c
import.c
magick-cli.c
magick-image.c
magick-property.c
magick-wand.c
mogrify.c
montage.c
operation.c
pixel-iterator.c
pixel-wand.c
script-token.c
stream.c
wand.c
wandcli.c
wand-view.c
)

ADD_LIBRARY(magickwand STATIC ${MAGICKWAND_BASE_SRCS})
TARGET_INCLUDE_DIRECTORIES(magickwand PUBLIC ${IMAGEMAGICK_BASE_PATH})
TARGET_INCLUDE_DIRECTORIES(magickwand PRIVATE  ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
# Set the MAGICKCORE_QUANTUM_DEPTH to 16, this also avoids build being unsuccessful on MSVC by using of preprocessor #warning
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
# Seems we need to define _LIB if we want to build statically
# Set the MAGICKCORE_SHAREARCH_BASEDIRNAME to "config", not sure if I should add it to the <config.h>
# Set LZMA_API_STATIC because we are using a static lzma library
TARGET_COMPILE_DEFINITIONS(magickwand PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE -D_LIB -DMAGICKCORE_SHAREARCH_BASEDIRNAME=\"config\" -DLZMA_API_STATIC)
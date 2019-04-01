PROJECT(filters C)

SET(FILTERS_BASE_SRCS
#sources
analyze.c
)

ADD_LIBRARY(filters STATIC ${FILTERS_BASE_SRCS})
TARGET_INCLUDE_DIRECTORIES(filters PUBLIC ${IMAGEMAGICK_BASE_PATH})
TARGET_INCLUDE_DIRECTORIES(filters PRIVATE  ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
# Set the MAGICKCORE_QUANTUM_DEPTH to 16, this also avoids build being unsuccessful on MSVC by using of preprocessor #warning
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
# Seems we need to define _LIB if we want to build statically
# Set LZMA_API_STATIC because we are using a static lzma library
TARGET_COMPILE_DEFINITIONS(filters PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE -D_LIB -DLZMA_API_STATIC)
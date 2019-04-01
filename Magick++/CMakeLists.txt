PROJECT(Magick++ CXX)

SET(MAGICKXX_BASE_SRCS
#headers
lib/Magick++.h 
lib/Magick++/Blob.h 
lib/Magick++/BlobRef.h 
lib/Magick++/CoderInfo.h 
lib/Magick++/Color.h 
lib/Magick++/Drawable.h 
lib/Magick++/Exception.h 
lib/Magick++/Functions.h 
lib/Magick++/Geometry.h 
lib/Magick++/Image.h 
lib/Magick++/ImageRef.h 
lib/Magick++/Include.h 
lib/Magick++/Montage.h 
lib/Magick++/Options.h 
lib/Magick++/Pixels.h 
lib/Magick++/ResourceLimits.h 
lib/Magick++/SecurityPolicy.h 
lib/Magick++/Statistic.h 
lib/Magick++/STL.h 
lib/Magick++/Thread.h 
lib/Magick++/TypeMetric.h
#sources
lib/Blob.cpp 
lib/BlobRef.cpp 
lib/CoderInfo.cpp 
lib/Color.cpp 
lib/Drawable.cpp 
lib/Exception.cpp 
lib/Functions.cpp 
lib/Geometry.cpp 
lib/Image.cpp 
lib/ImageRef.cpp 
lib/Montage.cpp 
lib/Options.cpp 
lib/Pixels.cpp 
lib/ResourceLimits.cpp 
lib/SecurityPolicy.cpp 
lib/Statistic.cpp 
lib/STL.cpp 
lib/Thread.cpp 
lib/TypeMetric.cpp 
)

ADD_LIBRARY(magick++ STATIC ${MAGICKXX_BASE_SRCS})
TARGET_INCLUDE_DIRECTORIES(magick++ PUBLIC ${IMAGEMAGICK_BASE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set the MAGICKCORE_QUANTUM_DEPTH to 16, this also avoids build being unsuccessful on MSVC by using of preprocessor #warning
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
# Seems we need to define _LIB if we want to build statically
TARGET_COMPILE_DEFINITIONS(magick++ PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE -D_LIB)
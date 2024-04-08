cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20.0)
project(delegates)

# CMake has a built-in FindImageMagick module that relies on pkg-config files
# * Adding here CONFIG is the best way to bypass it
# * Setting ImageMagick_ROOT=${MAGICK_INSTALL_PREFIX}/lib/cmake is the best
#   to set the ImageMagick directory without interfering with an (optional) conan
find_package(ImageMagick CONFIG)

# This allows using a shared ImageMagick library from a non-standard location
set(BUILD_RPATH ${CMAKE_LIBRARY_PATH})

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE "delegates.cpp")
target_include_directories(${PROJECT_NAME} PRIVATE ImageMagick::Magick++-7.Q16HDRI)
target_link_libraries(${PROJECT_NAME} PRIVATE ImageMagick::Magick++-7.Q16HDRI)

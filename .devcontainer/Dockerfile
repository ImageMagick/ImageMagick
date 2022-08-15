FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu-22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y autoconf autopoint binutils cmake gcc g++ gettext git gperf libtool make nasm pkg-config po4a

RUN mkdir /dependencies

WORKDIR /dependencies

RUN git clone --depth 1 https://gitlab.com/libtiff/libtiff
RUN git clone --depth 1 https://github.com/strukturag/libde265
RUN git clone --depth 1 https://github.com/strukturag/libheif
RUN git clone --depth 1 https://github.com/uclouvain/openjpeg
RUN git clone --depth 1 https://github.com/webmproject/libwebp
RUN git clone --depth 1 https://github.com/madler/zlib
RUN git clone --depth 1 https://github.com/libjpeg-turbo/libjpeg-turbo
RUN git clone --depth 1 https://github.com/xz-mirror/xz
RUN git clone --depth 1 https://github.com/libraw/libraw
RUN git clone --depth 1 https://github.com/pnggroup/libpng
RUN git clone --depth 1 https://github.com/mm2/Little-CMS
RUN git clone https://git.savannah.nongnu.org/r/freetype/freetype2.git/
RUN git -c http.sslVerify=false clone https://www.cl.cam.ac.uk/~mgk25/git/jbigkit

RUN curl https://raw.githubusercontent.com/ImageMagick/ImageMagick/main/Magick%2B%2B/fuzz/build_dependencies.sh -o build_dependencies.sh && \
    chmod +x build_dependencies.sh

ENV SRC=/dependencies
ENV WORK=/usr/local
ENV CFLAGS="-O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
ENV CXXFLAGS="-O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
RUN ./build_dependencies.sh
FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu-22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y autoconf autopoint binutils clang cmake gettext git gperf libc++-dev libc++abi-dev libtool make nasm pkg-config po4a

RUN mkdir /ImageMagick
RUN chown vscode.vscode /ImageMagick

USER vscode
RUN mkdir /ImageMagick/dependencies 
WORKDIR /ImageMagick/dependencies

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
RUN git clone --depth 1 https://github.com/freetype/freetype
RUN git clone --depth 1 https://gitlab.com/federicomenaquintero/bzip2.git
RUN git clone --depth 1 --recursive https://github.com/libjxl/libjxl
RUN git -c http.sslVerify=false clone https://www.cl.cam.ac.uk/~mgk25/git/jbigkit

RUN curl https://raw.githubusercontent.com/ImageMagick/ImageMagick/main/Magick%2B%2B/fuzz/build_dependencies.sh -o build_dependencies.sh && \
    chmod +x build_dependencies.sh

ENV SRC=/ImageMagick/dependencies
ENV WORK=/ImageMagick
ENV PATH=$PATH:/ImageMagick/bin
ENV CXX=clang++
ENV CC=clang
ENV CFLAGS="-O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
ENV CXXFLAGS="-O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -stdlib=libc++"
RUN ./build_dependencies.sh
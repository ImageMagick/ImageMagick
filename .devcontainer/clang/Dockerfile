FROM ubuntu:18.04@sha256:152dc042452c496007f07ca9127571cb9c29697f42acbfad72324b2bb2e43c98

RUN apt-get update && \
    apt-get install -y autoconf clang git libtool locales pkg-config && \
    locale-gen en_US.UTF-8

ENV CXX=clang++
ENV CC=clang
ENV CFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'

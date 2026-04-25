FROM ubuntu:22.04@sha256:962f6cadeae0ea6284001009daa4cc9a8c37e75d1f5191cf0eb83fe565b63dd7

RUN apt-get update && \
    apt-get install -y autoconf clang git libtool locales pkg-config && \
    locale-gen en_US.UTF-8

ENV CXX=clang++
ENV CC=clang
ENV CFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'

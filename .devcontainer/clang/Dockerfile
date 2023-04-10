FROM ubuntu:18.04

RUN apt-get update && \
    apt-get install -y autoconf clang git libtool locales pkg-config && \
    locale-gen en_US.UTF-8

ENV CXX=clang++
ENV CC=clang
ENV CFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-unused-function -Wno-incompatible-library-redeclaration'

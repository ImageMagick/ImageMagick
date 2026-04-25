FROM ubuntu:22.04@sha256:962f6cadeae0ea6284001009daa4cc9a8c37e75d1f5191cf0eb83fe565b63dd7

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y autoconf gcc g++ git libtool locales make pkg-config && \
    locale-gen en_US.UTF-8

ENV CFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'

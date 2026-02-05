FROM ubuntu:20.04@sha256:8feb4d8ca5354def3d8fce243717141ce31e2c428701f6682bd2fafe15388214

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y autoconf gcc g++ git libtool locales make pkg-config && \
    locale-gen en_US.UTF-8

ENV CFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'

FROM ubuntu:26.04@sha256:b7f48194d4d8b763a478a621cdc81c27be222ba2206ca3ca6bc42b49685f3d9e

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y autoconf gcc g++ git libtool locales make pkg-config && \
    locale-gen en_US.UTF-8

ENV CFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'
ENV CXXFLAGS='-Wall -Wextra -Werror -Wno-builtin-declaration-mismatch'

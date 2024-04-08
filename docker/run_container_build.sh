#!/bin/bash -x

DIRNAME=`dirname $0`

if [ -z "$1" ]; then
  echo -n "Select a container from the list: "
  find ${DIRNAME} -type f -name 'Dockerfile*' -printf %P
  echo
  exit 1
fi
CONTAINER=$1
shift

USER="--user `id -u`:`id -g`"

(
  cd ${DIRNAME}/..
  SOURCES="-v `pwd`:/src"
  cd docker
  docker build -t magick-${CONTAINER} -f Dockerfile.fedora . $@
  docker run ${SOURCES} ${USER} magick-${CONTAINER}
)

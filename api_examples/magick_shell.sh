#!/bin/sh
#
# Assumes the "magick" command has been installed
#
magick -size 100x100 xc:red \
       \( rose: -rotate -90 \) \
       +append   show:

#!/bin/bash
set -e

mkdir -p "$PACKAGE_NAME/bin" "$PACKAGE_NAME/etc" "$PACKAGE_NAME/share"
cp portable/usr/bin/magick "$PACKAGE_NAME/bin/"
cp -a portable/usr/etc/ImageMagick-7 "$PACKAGE_NAME/etc/"
cp -a portable/usr/share/ImageMagick-7 "$PACKAGE_NAME/share/"
cp portable/usr/lib/ImageMagick-*/config-*/configure.xml "$PACKAGE_NAME/etc/ImageMagick-7/"
cp ChangeLog.md LICENSE "$PACKAGE_NAME/"
cp config/sRGB.icm "$PACKAGE_NAME/sRGB.icc"
mkdir -p artifacts
COPYFILE_DISABLE=1 XZ_OPT="-9e" tar -cJf "artifacts/$PACKAGE_NAME.tar.xz" "$PACKAGE_NAME"

#
# Package name and versioning information for ImageMagick.
#
# This file is sourced by a Bourne shell (/bin/sh) script so it must
# observe Bourne shell syntax.
#
# Package base name
PACKAGE_NAME='ImageMagick'

#
# Date of last ChangeLog update
#
PACKAGE_CHANGE_DATE=`awk '/^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]/ { print substr($1,1,4) substr($1,6,2) substr($1,9,2); exit; }' ${srcdir}/ChangeLog`

#
# Package version.  This is is the numeric version suffix applied to
# PACKAGE_NAME (e.g. "1.0.0").
PACKAGE_VERSION='7.0.5'
PACKAGE_PERL_VERSION='7.05'
PACKAGE_LIB_VERSION="0x705"
PACKAGE_RELEASE="5"
PACKAGE_LIB_VERSION_NUMBER="7,0,5,${PACKAGE_RELEASE}"
PACKAGE_RELEASE_DATE_RAW=`date +%F`
PACKAGE_RELEASE_DATE_REPRODUCIBLE="${PACKAGE_CHANGE_DATE}"
PACKAGE_STRING="$PACKAGE_NAME $PACKAGE_VERSION"

#
# Package version addendum.  This is an arbitrary suffix (if any) appended
# to the package version. (e.g. "beta1")
PACKAGE_VERSION_ADDENDUM="-${PACKAGE_RELEASE}"

#
# Versions are denoted using a standard triplet of integers:
# MAJOR.MINOR.PATCH. The basic intent is that MAJOR versions are
# incompatible, large-scale upgrades of the API. MINOR versions retain
# source and binary compatibility with older minor versions, and changes
# in the PATCH level are perfectly compatible, forwards and backwards.
# See http://apr.apache.org/versioning.html.
#
# PLEASE NOTE that doing a SO BUMP aka raising the CURRENT REVISION
# could be avoided using libversioning aka map files.  You MUST change .map
# files if you raise these versions.
MAGICK_LIBRARY_CURRENT=2
MAGICK_LIBRARY_REVISION=0
MAGICK_LIBRARY_AGE=0

# magick++
MAGICKPP_LIBRARY_CURRENT=2
MAGICKPP_LIBRARY_REVISION=0
MAGICKPP_LIBRARY_AGE=0

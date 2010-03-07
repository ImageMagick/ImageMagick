#
# Package name and versioning information for ImageMagick.
#
# This file is sourced by a Bourne shell (/bin/sh) script so it must
# observe Bourne shell syntax.
#
# Package base name
PACKAGE_NAME='ImageMagick'

#
# Package version.  This is is the numeric version suffix applied to
# PACKAGE_NAME (e.g. "1.0.0").
PACKAGE_VERSION='6.6.0'
PACKAGE_LIB_VERSION="0x660"
PACKAGE_RELEASE="4"
PACKAGE_LIB_VERSION_NUMBER="6,6,0,${PACKAGE_RELEASE}"
PACKAGE_RELEASE_DATE=`date +%F`
PACKAGE_STRING="$PACKAGE_NAME $PACKAGE_VERSION"

#
# Date of last ChangeLog update
#
PACKAGE_CHANGE_DATE=`awk '/^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]/ { print substr($1,1,4) substr($1,6,2) substr($1,9,2); exit; }' ${srcdir}/ChangeLog`

#
# Package version addendum.  This is an arbitrary suffix (if any) appended
# to the package version. (e.g. "beta1")
PACKAGE_VERSION_ADDENDUM="-${PACKAGE_RELEASE}"

#
# Libtool library revision control info: See the libtool documentation under
# the heading "Libtool's versioning system" in order to understand the meaning
# of these fields.
#
# Here are a set of rules to help you update your library version
# information:
#
#   If there is any interface change, increment CURRENT (major).  If that
#   interface change does not break upward compatibility (i.e. it is an
#   addition), increment AGE( micro), Otherwise AGE is reset to 0. If CURRENT
#   has changed, REVISION (minor) is set to 0, otherwise REVISION is
#   incremented.

MAGICK_LIBRARY_CURRENT=3
MAGICK_LIBRARY_REVISION=0
MAGICK_LIBRARY_AGE=0

#
# Package versioning information for ImageMagick.
#
# This file is sourced by a Bourne shell (/bin/sh) script so it must
# observe Bourne shell syntax.
#

#
# Package release date.
#
PACKAGE_RELEASE_DATE_RAW=`date +%F`
PACKAGE_CHANGE_DATE=`awk '/^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]/ { print substr($1,1,4) substr($1,6,2) substr($1,9,2); exit; }' ${srcdir}/ChangeLog`
PACKAGE_RELEASE_DATE_REPRODUCIBLE="${PACKAGE_CHANGE_DATE}"

#
# If the library source code has changed at all since the last update,
# increment revision (‘c:r:a’ becomes ‘c:r+1:a’).  If any interfaces have been
# added, removed, or changed since the last update, increment current, and set
# revision to 0.  If any interfaces have been added since the last public
# release, then increment age.  If any interfaces have been removed or changed
# since the last public release, then set age to 0.
#
# PLEASE NOTE that doing a SO BUMP aka raising the CURRENT REVISION
# could be avoided using libversioning aka map files.  You MUST change .map
# files if you raise these versions.
#
# Bump the minor release # whenever there is an SOVersion bump.
MAGICK_LIBRARY_CURRENT=8
MAGICK_LIBRARY_REVISION=0
MAGICK_LIBRARY_AGE=0

# magick++
MAGICKPP_LIBRARY_CURRENT=4
MAGICKPP_LIBRARY_REVISION=0
MAGICKPP_LIBRARY_AGE=0

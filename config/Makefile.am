#  Copyright 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for Magick ImageMagick configuration files.

# Where architecture-independent configuration files get installed
# (share/ImageMagick-version)
configsharedir = $(SHARE_PATH)
configshare_DATA = \
  config/english.xml \
  config/francais.xml \
  config/locale.xml

# Where architecture-dependent configuration files get installed
# (share/arch/ImageMagick-version)
configsharearchdir = $(SHAREARCH_PATH)
configsharearch_DATA = \
  config/configure.xml

# Where architecture-dependent configuration files get installed
# (share/ImageMagick-version)
configlibdir =  $(CONFIGURE_PATH)
configlib_DATA = \
  config/colors.xml \
  config/delegates.xml \
  config/log.xml \
  config/mime.xml \
  config/policy.xml \
  config/quantization-table.xml \
  config/thresholds.xml \
  config/type.xml \
  config/type-apple.xml \
  config/type-dejavu.xml \
  config/type-ghostscript.xml \
  config/type-urw-base35.xml \
  config/type-windows.xml

CONFIG_EXTRA_DIST = \
  config/cmyk.icm \
  config/colors.xml \
  config/config.h.in \
  config/delegates.xml.in \
  config/english.xml \
  config/francais.xml \
  config/ImageMagick.rc \
  config/ImageMagick.rdf.in \
  config/install-sh \
  config/lndir.sh \
  config/locale.md \
  config/locale.xml \
  config/log.xml \
  config/mime.xml \
  config/policy-limited.xml \
  config/policy-open.xml \
  config/policy-secure.xml \
  config/policy-websafe.xml \
  config/quantization-table.xml \
  config/sRGB.icm \
  config/thresholds.xml \
  config/type-apple.xml.in \
  config/type-dejavu.xml.in \
  config/type-ghostscript.xml.in \
  config/type-urw-base35.xml.in \
  config/type-windows.xml.in \
  config/type.xml.in

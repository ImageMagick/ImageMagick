# Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
# dedicated to making software imaging solutions freely available.
#
# You may not use this file except in compliance with the License.
# obtain a copy of the License at
#
#   https://imagemagick.org/script/license.php
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  Makefile for the ImageMagick validation suite.
TESTS_CPPFLAGS = $(AM_CPPFLAGS)

TESTS_CHECK_PGRMS = \
  tests/validate \
  tests/drawtest \
  tests/wandtest

tests_validate_SOURCES  = tests/validate.c tests/validate.h
tests_validate_CPPFLAGS = $(TESTS_CPPFLAGS)
tests_validate_LDFLAGS  = $(LDFLAGS)
tests_validate_LDADD    = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(MATH_LIBS)

tests_drawtest_SOURCES  = tests/drawtest.c
tests_drawtest_CPPFLAGS = $(TESTS_CPPFLAGS)
tests_drawtest_LDFLAGS  = $(LDFLAGS)
tests_drawtest_LDADD    = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)

tests_wandtest_SOURCES  = tests/wandtest.c
tests_wandtest_CPPFLAGS = $(TESTS_CPPFLAGS)
tests_wandtest_LDFLAGS  = $(LDFLAGS)
tests_wandtest_LDADD    = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)

TESTS_XFAIL_TESTS = 

TESTS_TESTS = \
  tests/cli-colorspace.tap \
  tests/cli-pipe.tap \
  tests/validate-colorspace.tap \
  tests/validate-compare.tap \
  tests/validate-composite.tap \
  tests/validate-convert.tap \
  tests/validate-formats-disk.tap \
  tests/validate-formats-map.tap \
  tests/validate-formats-memory.tap \
  tests/validate-identify.tap \
  tests/validate-import.tap \
  tests/validate-montage.tap \
  tests/validate-stream.tap \
  tests/drawtest.tap \
  tests/wandtest.tap

TESTS_EXTRA_DIST = \
  tests/common.shi \
  tests/rose.pnm \
  tests/input_256c.miff \
  tests/input_bilevel.miff \
  tests/input_gray.miff \
  tests/input_truecolor.miff \
  tests/sequence.miff \
  $(TESTS_TESTS)

TESTS_CLEANFILES = \
  tests/*_out.*

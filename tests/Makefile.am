# Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
# dedicated to making software imaging solutions freely available.
#
# You may not use this file except in compliance with the License.
# obtain a copy of the License at
#
#   http://www.imagemagick.org/script/license.php
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  Makefile for the ImageMagick validation suite.

TESTS_CHECK_PGRMS = \
	tests/validate

tests_validate_SOURCES = tests/validate.c tests/validate.h
tests_validate_CPPFLAGS = $(AM_CPPFLAGS)
tests_validate_LDADD = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)

TESTS_XFAIL_TESTS = 

TESTS_TESTS = \
	tests/validate-compare.sh \
	tests/validate-composite.sh \
	tests/validate-convert.sh \
	tests/validate-identify.sh \
	tests/validate-import.sh \
	tests/validate-montage.sh \
	tests/validate-pipe.sh \
	tests/validate-stream.sh \
	tests/validate-formats-in-memory.sh \
	tests/validate-formats-on-disk.sh

TESTS_EXTRA_DIST = \
	tests/common.sh \
	$(TESTS_TESTS)

TESTS_CLEANFILES = 


# get includes and libraries from source directory
SRC=$(shell pwd )/../..
CFLAGS=-I$(SRC) -L$(SRC)/MagickWand/.libs -L$(SRC)/MagickCore/.libs

# get includes and libraries from installed ImageMagick-devel Package
#CFLAGS=-I/usr/include/ImageMagick

LDLIBS=-lMagickWand -lMagickCore

files=$(wildcard *.c)
tests=$(files:%.c=%)

all: $(tests)

script-token-test: script-token-test.c ../script-token.[ch]
	$(CC) -o script-token-test script-token-test.c

clean:
	rm -f $(tests)

test_script:
	script-token-test.sh | diff script-token-test-results.txt -


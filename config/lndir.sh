#! /bin/sh

# lndir - create shadow link tree
#
# Time stamp <89/11/28 18:56:54 gildea>
# By Stephen Gildea <gildea@bbn.com> based on
#  XConsortium: lndir.sh,v 1.1 88/10/20 17:37:16 jim Exp
#
# Modified slightly for ImageMagick by Bob Friesenhahn, 1999
#
# Used to create a copy of the a directory tree that has links for all
# non- directories.  If you are building the distribution on more than
# one machine, you should use this script.
#
# If your master sources are located in /usr/local/src/X and you would like
# your link tree to be in /usr/local/src/new-X, do the following:
#
# 	%  mkdir /usr/local/src/new-X
#	%  cd /usr/local/src/new-X
# 	%  lndir ../X
#
# Note: does not link files beginning with "."  Is this a bug or a feature?
#
# Improvements over R3 version:
#   Allows the fromdir to be relative: usually you want to say "../dist"
#   The name is relative to the todir, not the current directory.
#
# Bugs in R3 version fixed:
#   Do "pwd" command *after* "cd $DIRTO".
#   Don't try to link directories, avoiding error message "<dir> exists".
#   Barf with Usage message if either DIRFROM *or* DIRTO is not a directory.

USAGE="Usage: $0 fromdir [todir]"

if [ $# -lt 1 -o $# -gt 2 ]
then
    echo "$USAGE"
    exit 1
fi

DIRFROM=$1

if [ $# -eq 2 ];
then
    DIRTO=$2
else
    DIRTO=.
fi

if [ ! -d $DIRTO ]
then
    echo "$0: $DIRTO is not a directory"
    echo "$USAGE"
    exit 2
fi

cd $DIRTO

if [ ! -d $DIRFROM ]
then
    echo "$0: $DIRFROM is not a directory"
    echo "$USAGE"
    exit 2
fi

pwd=`pwd`

if [ `(cd $DIRFROM; pwd)` = $pwd ]
then
    echo "$pwd: FROM and TO are identical!"
    exit 1
fi

for file in `ls $DIRFROM`
do
    if [ ! -d $DIRFROM/$file ]
    then
	test -r $file || ln -s $DIRFROM/$file .
    else
	#echo $file:
	test -d $file || mkdir $file && chmod 777 $file
	(cd $file
	pwd=`pwd`
	case "$DIRFROM" in
	    /*) ;;
	    *)  DIRFROM=../$DIRFROM ;;
	esac
	if [ `(cd $DIRFROM/$file; pwd)` = $pwd ]
	then
	    echo "$pwd: FROM and TO are identical!"
	    exit 1
	fi
	$0 $DIRFROM/$file
	)
    fi
done

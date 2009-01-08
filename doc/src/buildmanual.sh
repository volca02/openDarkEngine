#!/bin/sh
#$Id$

die() {
	echo $@
	exit 1
}

[ "x$1" = "x" ] && die "buildmanual.sh: No source path specified"
[ "x$2" = "x" ] && die "buildmanual.sh: No destination path specified"

SRC_DIR=$1
DST_DIR=$2

mkdir -p $DST_DIR
mkdir -p $DST_DIR/img/

# Convert all the .dot files to png
for f in $SRC_DIR/*.dot;
do
        dot -T png -o $DST_DIR/img/`basename $f .dot`.png $f
done

cd $DST_DIR/

texi2html -output ./ -split=section -top_file=index.html -init_file $SRC_DIR/opdetexi2html.init $SRC_DIR/manual.texi -P .

#!/bin/bash

mkdir -p bld

#
# PDF
#
pandoc \
 -V geometry:margin=0.6in \
 pbf2sqlite.md \
 --pdf-engine=xelatex \
 --toc \
 -o ./bld/pbf2sqlite.pdf

#
# HTML
#
pandoc \
 --standalone \
 --embed-resources \
 --metadata title="pbf2sqlite" \
 --toc \
 --css=custom.css \
 pbf2sqlite.md \
 -o ./bld/pbf2sqlite.html

#
# manpage
#
rm -f ./bld/pbf2sqlite.1.gz
pandoc \
 -s -f markdown -t man \
 pbf2sqlite.md \
 -o ./bld/pbf2sqlite.1
gzip ./bld/pbf2sqlite.1

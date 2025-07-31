#!/bin/bash

mkdir -p ../build

#
# PDF
#
pandoc \
 -V geometry:margin=0.6in \
 pbf2sqlite.md \
 --pdf-engine=xelatex \
 --toc \
 -o ../build/pbf2sqlite.pdf

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
 -o ../build/pbf2sqlite.html

#
# manpage
#
rm -f ../build/pbf2sqlite.1.gz
pandoc \
 -s -f markdown -t man \
 pbf2sqlite.md \
 -o ../build/pbf2sqlite.1
gzip ../build/pbf2sqlite.1

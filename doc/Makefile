#
# Makefile for generating documentation using Pandoc
#

# Input and output files
SRC = pbf2sqlite.md
PDF_DOC = ../build/pbf2sqlite.pdf
HTML_DOC = ../build/pbf2sqlite.html
MAN_DOC = ../build/pbf2sqlite.1

# Targets
all: pdf html man

pdf:
	mkdir -p $(dir $(PDF_DOC))
	pandoc \
     -V geometry:margin=0.6in \
     $(SRC) \
     --pdf-engine=xelatex \
     --toc \
     -o $(PDF_DOC)

html:
	mkdir -p $(dir $(HTML_DOC))
	pandoc \
     --standalone \
     --embed-resources \
     --metadata title="pbf2sqlite" \
     --toc \
     --css=custom.css \
     $(SRC) \
     -o $(HTML_DOC)

man:
	mkdir -p $(dir $(MAN_DOC))
	rm -f $(MAN_DOC).gz
	pandoc \
     -s -f markdown -t man \
     $(SRC) \
     -o $(MAN_DOC)
	gzip $(MAN_DOC)

clean:
	rm -f $(PDF_DOC) $(HTML_DOC) $(MAN_DOC).gz

.PHONY: all pdf html man clean

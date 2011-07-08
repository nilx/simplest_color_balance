# Copyright 2009-2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

# source code, C language
CSRC	= io_png.c balance_lib.c colorspace_lib.c \
		balance_rgb.c balance_hsl.c balance_hsi_bounded.c
# source code, all languages (only C here)
SRC	= $(CSRC)
# object files (partial compilation)
OBJ	= $(CSRC:.c=.o)
# binary executable programs
BIN	= balance_rgb balance_hsl balance_hsi_bounded

# standard C compiler optimization options
COPT	= -O3 -funroll-loops -fomit-frame-pointer -ffast-math
# complete C compiler options
CFLAGS	= -ansi -pedantic -Wall -Wextra -Werror -pipe $(COPT)
# linker options
LDFLAGS	+= -lpng

# default target: the binary executable programs
default: $(BIN)

# partial C compilation xxx.c -> xxx.o
%.o	: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

# final link
balance_rgb	: io_png.o balance_lib.o balance_rgb.o
	$(CC) $^ $(LDFLAGS) -o $@
balance_hsl	: io_png.o balance_lib.o colorspace_lib.o balance_hsl.o
	$(CC) $^ $(LDFLAGS) -o $@
balance_hsi_bounded	: io_png.o balance_lib.o balance_hsi_bounded.o
	$(CC) $^ $(LDFLAGS) -o $@

# cleanup
.PHONY	: clean distclean
clean	:
	$(RM) $(OBJ)
distclean	: clean
	$(RM) $(BIN)
	$(RM) -r srcdoc

################################################
# extra tasks

PROJECT	= simplest_color_balance
DATE	= $(shell date -u +%Y%m%d)
RELEASE_TAG   = 0.$(DATE)

.PHONY	: srcdoc lint beautify test release
# source documentation
srcdoc	: $(SRC)
	doxygen doc/doxygen.conf
# code cleanup
beautify	: $(CSRC)
	for FILE in $^; do \
		expand $$FILE | sed 's/[ \t]*$$//' > $$FILE.$$$$ \
		&& indent -kr -i4 -l78 -nut -nce -sob -sc \
			$$FILE.$$$$ -o $$FILE \
		&& rm $$FILE.$$$$; \
	done
# static code analysis
lint	: $(CSRC)
	for FILE in $^; do \
		clang --analyze -ansi -I. $$FILE || exit 1; done;
	for FILE in $^; do \
		splint -ansi-lib -weak -I. $$FILE || exit 1; done;
	$(RM) *.plist
# code tests
test	: $(CSRC)
	sh -e test/run.sh && echo SUCCESS || ( echo ERROR; return 1)
# release tarball
release	: beautify lint test distclean
	git archive --format=tar --prefix=$(PROJECT)-$(RELEASE_TAG)/ HEAD \
	        | gzip > ../$(PROJECT)-$(RELEASE_TAG).tar.gz

# Copyright 2009-2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

# source code, C language
SRC	= io_png.c balance_lib.c colorbalance_lib.c balance.c
# header-only code
HDR	= debug.h
# object files (partial compilation)
OBJ	= $(SRC:.c=.o)
# binary executable programs
BIN	= balance

# standard C compiler optimization options
COPT	= -O3
# complete C compiler options
CFLAGS	= -ansi -pedantic -Wall -Wextra -Werror -pipe $(COPT)
# preprocessor options
CPPFLAGS	= -DNDEBUG
# linker options
LDFLAGS	= -lpng

# default target: the binary executable programs
default: $(BIN)

# partial C compilation xxx.c -> xxx.o
%.o	: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

# final link
balance	: io_png.o balance_lib.o colorbalance_lib.o balance.o
	$(CC) $^ $(LDFLAGS) -o $@

# cleanup
.PHONY	: clean distclean
clean	:
	$(RM) $(OBJ)
distclean	: clean
	$(RM) $(BIN)
	$(RM) -r srcdoc

################################################
# dev tasks
PROJECT	= simplest_color_balance
-include	makefile.dev

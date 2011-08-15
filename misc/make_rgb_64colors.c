#!/usr/bin/tcc -run -lpng -Wall -Werror
/*
 * Copyright 2011 IPOL Image Processing On Line http://www.ipol.im/
 * Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty provided
 * the copyright notice and this notice are preserved.  This file is
 * offered as-is, without any warranty.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../io_png.c"

int main()
{
    size_t nx = 64 * 8;
    size_t ny = 64 * 8;
    size_t size = nx * ny;
    size_t i, x, y;
    unsigned char *data, *r, *g, *b;

    data = (unsigned char *) malloc(size * 3 * sizeof(unsigned char));
    r = data;
    g = data + size;
    b = data + 2 * size;

    for (i = 0; i < size; i++) {
        x = i % (64 * 8);
        y = i / (64 * 8);
        r[i] = x % 64;
        g[i] = y % 64;
        b[i] = (x / 64) + 8 * (y / 64);
        r[i] *= 4;
        g[i] *= 4;
        b[i] *= 4;
    }

    io_png_write_u8("-", data, nx, ny, 3);
    free(data);

    return EXIT_SUCCESS;
}

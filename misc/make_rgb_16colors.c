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
    size_t nx = 16 * 4;
    size_t ny = 16 * 4;
    size_t size = nx * ny;
    size_t i, x, y;
    unsigned char *data, *r, *g, *b;

    data = (unsigned char *) malloc(size * 3 * sizeof(unsigned char));
    r = data;
    g = data + size;
    b = data + 2 * size;

    for (i = 0; i < size; i++) {
        x = i % (16 * 4);
        y = i / (16 * 4);
        r[i] = x % 16;
        g[i] = y % 16;
        b[i] = (x / 16) + 4 * (y / 16);
        r[i] *= 16;
        g[i] *= 16;
        b[i] *= 16;
    }

    io_png_write_u8("-", data, nx, ny, 3);
    free(data);

    return EXIT_SUCCESS;
}

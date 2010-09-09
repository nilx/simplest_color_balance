/*
 * Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file normalize_histo.c
 * @brief histogram-based normalization
 *
 * The input image is normalized to [0-255], saturating a percentage
 * of the pixels at the beginning and end of the color space, using a
 * histogram algorithm.
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_png.h"
#include "normalize_histo_lib.h"

/**
 * @brief main function call
 *
 * The saturation is processed half at both ends of the histogram.
 */
int main(int argc, char *const *argv)
{
    float s;                    /* saturated percentage */
    size_t nx, ny;              /* data size */
    unsigned char *data;        /* input/output data */
    int channel;

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1]))
    {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (4 != argc)
    {
        fprintf(stderr, "usage : %s S in.png out.png\n", argv[0]);
        fprintf(stderr, "        S saturated pixels percentage [0...100[\n");
        return EXIT_FAILURE;
    }

    /* flattening percentage */
    s = atof(argv[1]);
    if (0. > s || 100. <= s)
    {
        fprintf(stderr, "the saturation percentage must be in [0..100[\n");
        return EXIT_FAILURE;
    }

    /* read the PNG image */
    if (NULL == (data = read_png_u8_rgb(argv[2], &nx, &ny)))
    {
        fprintf(stderr, "the image could not be properly read\n");
        return EXIT_FAILURE;
    }

    /*
     * do normalization on RGB channels
     * we saturate s% pixels, half on both sides of the histogram
     */
    for (channel = 0; channel < 3; channel++)
        (void) normalize_histo_u8(data + channel * nx * ny, nx * ny, 0, 255,
                                  nx * ny * (s / 100. / 2.),
                                  nx * ny * (s / 100. / 2.));

    /* write the PNG image */
    write_png_u8(argv[3], data, nx, ny, 3);

    free(data);

    return EXIT_SUCCESS;
}

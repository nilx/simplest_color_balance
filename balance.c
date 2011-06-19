/*
 * Copyright 2009-2011 IPOL Image Processing On Line http://www.ipol.im/
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
 * @file balance.c
 * @brief simplest color balance
 *
 * The input image is normalized to [0-UCHAR_MAX], saturating a percentage
 * of the pixels at the beginning and end of the color space, using a
 * histogram algorithm.
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 * @author Jose-Luis Lisani <joseluis.lisani@uib.es>
 * @author Catalina Sbert <catalina.sbert@uib.es>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "io_png.h"
#include "balance_lib.h"

/**
 * @brief main function call
 *
 * The saturation is processed half at both ends of the histogram.
 */
int main(int argc, char *const *argv)
{
    float s1, s2;               /* saturated percentage */
    size_t nx, ny, size;        /* data size */
    size_t nb_min, nb_max;      /* number of saturated pixels */
    unsigned char *data;        /* input/output data */
    unsigned char *data_tmp;    /* duplicate data */

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1])) {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (6 != argc) {
        fprintf(stderr, "usage : %s S1 S2 in.png out.png out2.png\n",
                argv[0]);
        fprintf(stderr,
                "        S1 and S2 saturated pixels percentage [0...100[\n");
        return EXIT_FAILURE;
    }

    /* saturation percentage */
    s1 = atof(argv[1]);
    s2 = atof(argv[2]);
    if (0. > s1 || 100. <= s1 || 0. > s2 || 100. <= s2) {
        fprintf(stderr, "the saturation percentages must be in [0..100[\n");
        return EXIT_FAILURE;
    }

    /* read the PNG image */
    if (NULL == (data = io_png_read_u8_rgb(argv[3], &nx, &ny))) {
        fprintf(stderr, "the image could not be properly read\n");
        return EXIT_FAILURE;
    }
    size = nx * ny;
    data_tmp = malloc(3 * size * sizeof(unsigned char));
    memcpy(data_tmp, data, 3 * size * sizeof(unsigned char));

    /*
     * we saturate nb_min pixels on bottom
     * and nb_max pixels on the top of the histogram
     */
    nb_min = size * (s1 / 100.);
    nb_max = size * (s2 / 100.);

    /*
     * do normalization on RGB channels
     */
    (void) balance_u8(data, size, nb_min, nb_max);
    (void) balance_u8(data + size, size, nb_min, nb_max);
    (void) balance_u8(data + 2 * size, size, nb_min, nb_max);

    /* write the PNG image and restore the saved input data */
    io_png_write_u8(argv[4], data, nx, ny, 3);
    free(data);
    data = data_tmp;

    /*
     * do normalization on intensity channel
     */
    (void) balance_intensity_u8(data, size, nb_min, nb_max);

    /* write the PNG image */
    io_png_write_u8(argv[5], data, nx, ny, 3);
    free(data);

    return EXIT_SUCCESS;
}

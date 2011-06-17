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
 * @file normalize_sort.c
 * @brief sort-based normalization
 *
 * The input image is normalized to [0-255], saturating a percentage
 * of the pixels at the beginning and end of the color space, using a
 * sorting algorithm.
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "io_png.h"

/**
 * @brief unsigned char comparison
 */
static int cmp_u8(const void *a, const void *b)
{
    return (int) (*(const unsigned char *) a - *(const unsigned char *) b);
}

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
    unsigned char *tmpdata;     /* temporary sort data */
    int channel;

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1])) {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (4 != argc) {
        fprintf(stderr, "usage : %s S in.png out.png\n", argv[0]);
        fprintf(stderr, "        S saturated pixels percentage [0...100[\n");
        return EXIT_FAILURE;
    }

    /* flattening percentage */
    s = atof(argv[1]);
    if (0. > s || 100. <= s) {
        fprintf(stderr, "the saturation percentage must be in [0..100[\n");
        return EXIT_FAILURE;
    }

    /* read the TIFF image */
    if (NULL == (data = io_png_read_u8_rgb(argv[2], &nx, &ny))) {
        fprintf(stderr, "the image could not be properly read\n");
        return EXIT_FAILURE;
    }

    /* allocate a temporary array for sorting */
    if (NULL == (tmpdata = (unsigned char *)
                 malloc(nx * ny * sizeof(unsigned char)))) {
        fprintf(stderr, "allocation error\n");
        free(data);
        return EXIT_FAILURE;
    }

    /*
     * do normalization on RGB channels
     * we saturate s% pixels, half on both sides of the histogram
     */
    for (channel = 0; channel < 3; channel++) {
        float scale;
        unsigned char min, max;
        unsigned char norm[UCHAR_MAX + 1];
        size_t i;
        unsigned char *data_ptr, *data_end;

        /* copy the data and sort */
        memcpy(tmpdata, data + channel * nx * ny,
               nx * ny * sizeof(unsigned char));
        qsort(tmpdata, nx * ny, sizeof(unsigned char), &cmp_u8);

        /* get the min/max */
        i = (size_t) (nx * ny * (s / 100. / 2.));
        min = tmpdata[i];
        max = tmpdata[nx * ny - i - 1];

        /* rescale */
        /* max <= min : constant output */
        if (max <= min) {
            for (i = 0; i < UCHAR_MAX + 1; i++)
                norm[i] = 128;
        }
        else {
            /*
             * build an bounded affine normalization table
             * such that norm(min) = target_min
             *           norm(max) = target_max
             * norm(x) = (x - min) * (t_max - t_min) / (max - min) + t_min
             */
            scale = 255. / (float) (max - min);
            for (i = 0; i < min; i++)
                norm[i] = 0;
            for (i = min; i < max; i++)
                norm[i] = (unsigned char) ((i - min) * scale);
            for (i = max; i < UCHAR_MAX + 1; i++)
                norm[i] = 255;

            /* use the normalization table to transform the data */
            data_ptr = data + channel * nx * ny;
            data_end = data_ptr + nx * ny;
            while (data_ptr < data_end) {
                *data_ptr = norm[(size_t) (*data_ptr)];
                data_ptr++;
            }
        }
    }
    /* write the TIFF image */
    io_png_write_u8(argv[3], data, nx, ny, 3);

    free(data);
    free(tmpdata);

    return EXIT_SUCCESS;
}

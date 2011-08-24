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
 * @brief command-line handler for the color balance algorithms
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
#include "colorbalance_lib.h"

/**
 * @brief main function call
 */
int main(int argc, char *const *argv)
{
    float smin, smax;           /* saturated percentage */
    size_t nx, ny, size, i;     /* data size and index */

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1])) {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (6 != argc) {
        fprintf(stderr, "usage : %s mode Smin Smax in.png out.png\n",
                argv[0]);
        fprintf(stderr, "        mode is rgb or irgb\n");
        fprintf(stderr, "          (see README.txt for details)\n");
        fprintf(stderr, "        Smin and Smax are percentage of pixels\n");
        fprintf(stderr, "          saturated to min and max,\n");
        fprintf(stderr, "          in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* saturation percentage */
    smin = atof(argv[2]);
    smax = atof(argv[3]);
    if (0. > smin || 100. <= smin || 0. > smax || 100. <= smax) {
        fprintf(stderr, "the saturation percentages must be in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* select the color mode */
    if (0 == strcmp(argv[1], "rgb")) {
        unsigned char *rgb;     /* input/output data */

        /* read the PNG image in [0-UCHAR_MAX] */
        if (NULL == (rgb = io_png_read_u8_rgb(argv[4], &nx, &ny))) {
            fprintf(stderr, "the image could not be properly read\n");
            return EXIT_FAILURE;
        }
        size = nx * ny;

        /* execute the algorithm */
        (void) colorbalance_rgb_u8(rgb, size,
                                   size * (smin / 100.),
                                   size * (smax / 100.));

        /* write the PNG image from [0,UCHAR_MAX] and free the memory space */
        io_png_write_u8(argv[5], rgb, nx, ny, 3);
        free(rgb);
    }
    else if (0 == strcmp(argv[1], "irgb")) {
        float *rgb;             /* input/output data */

        /* read the PNG image in [0-1] */
        /** @todo correct io_png API */
        if (NULL == (rgb = io_png_read_f32_rgb(argv[4], &nx, &ny))) {
            fprintf(stderr, "the image could not be properly read\n");
            return EXIT_FAILURE;
        }
        size = nx * ny;
        for (i = 0; i < 3 * size; i++)
            rgb[i] /= 255.;

        /* execute the algorithm */
        (void) colorbalance_irgb_f32(rgb, size,
                                     size * (smin / 100.),
                                     size * (smax / 100.));

        /* write the PNG image from [0,1] and free the memory space */
        /** @todo correct io_png API */
        for (i = 0; i < 3 * size; i++)
            rgb[i] *= 255.;
        io_png_write_f32(argv[5], rgb, nx, ny, 3);
        free(rgb);
    }
    else {
        fprintf(stderr, "mode must be rgb or irgb\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

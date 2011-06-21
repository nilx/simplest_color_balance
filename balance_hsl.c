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
 * @file balance_hsl.c
 * @brief simplest color balance in th HSL color space
 *
 * The input image is normalized by affine transformation on the L
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis.
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
#include "colorspace_lib.h"

/**
 * @brief main function call
 */
int main(int argc, char *const *argv)
{
    float smin, smax;           /* saturated percentage */
    size_t nx, ny, size;        /* data size */
    size_t nb_min, nb_max;      /* number of saturated pixels */
    float *rgb, *hsl;           /* input/output data */

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1])) {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (5 != argc) {
        fprintf(stderr, "usage : %s Sb Sw in.png out.png\n", argv[0]);
        fprintf(stderr, "        Sb and Sw are percentage of pixels\n");
        fprintf(stderr, "        saturated to black and white, in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* saturation percentage */
    smin = atof(argv[1]);
    smax = atof(argv[2]);
    if (0. > smin || 100. <= smin || 0. > smax || 100. <= smax) {
        fprintf(stderr, "the saturation percentages must be in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* read the PNG image */
    if (NULL == (rgb = io_png_read_f32_rgb(argv[3], &nx, &ny))) {
        fprintf(stderr, "the image could not be properly read\n");
        return EXIT_FAILURE;
    }
    size = nx * ny;

    /*
     * we saturate nb_min pixels on bottom
     * and nb_max pixels on the top of the histogram
     */
    nb_min = size * (smin / 100.);
    nb_max = size * (smax / 100.);

    /*
     * do normalization on the L axis
     */
    /* convert to HSL */
    hsl = (float *) malloc(3 * size * sizeof(float));
    rgb2hsl(rgb, hsl, size);
    /* normalize the L channel */
    (void) balance_f32(hsl + 2 * size, size, nb_min, nb_max);
    /* convert back to HSL */
    hsl2rgb(hsl, rgb, size);
    free(hsl);

    /* write the PNG image and free the memory space */
    io_png_write_f32(argv[4], rgb, nx, ny, 3);
    free(rgb);

    return EXIT_SUCCESS;
}

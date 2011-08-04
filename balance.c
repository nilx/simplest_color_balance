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
 * @brief various color balance schemes
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
    float *rgb;                 /* input/output data */

    /* "-v" option : version info */
    if (2 <= argc && 0 == strcmp("-v", argv[1])) {
        fprintf(stdout, "%s version " __DATE__ "\n", argv[0]);
        return EXIT_SUCCESS;
    }
    /* wrong number of parameters : simple help info */
    if (6 != argc) {
        fprintf(stderr, "usage : %s mode Sb Sw in.png out.png\n", argv[0]);
        fprintf(stderr, "        mode is rgb, hsl, hsv or hsi\n");
        fprintf(stderr, "        Sb and Sw are percentage of pixels\n");
        fprintf(stderr, "        saturated to black and white, in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* saturation percentage */
    smin = atof(argv[2]);
    smax = atof(argv[3]);
    if (0. > smin || 100. <= smin || 0. > smax || 100. <= smax) {
        fprintf(stderr, "the saturation percentages must be in [0-100[\n");
        return EXIT_FAILURE;
    }

    /* read the PNG image */
    if (NULL == (rgb = io_png_read_f32_rgb(argv[4], &nx, &ny))) {
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

    /* select the color mode */
    if (0 == strcmp(argv[1], "rgb")) {
        /*
         * simplest color balance on RGB channels
         *
         * The input image is normalized by affine transformation on
         * each RGB channel, saturating a percentage of the pixels at
         * the beginning and end of the color space on each channel.
         */
        size_t i;
        /* normalize the RGB channels */
        (void) balance_f32(rgb, size, nb_min, nb_max);
        (void) balance_f32(rgb + size, size, nb_min, nb_max);
        (void) balance_f32(rgb + 2 * size, size, nb_min, nb_max);
        /* back to [0, 255] */
        for (i = 0; i < 3 * size; i++)
            rgb[i] *= 255.;
    }
    else if (0 == strcmp(argv[1], "hsl")) {
        /*
         * simplest color balance in the HSL color space, L channel
         *
         * The input image is normalized by affine transformation on
         * the L axis, saturating a percentage of the pixels at the
         * beginning and end of the axis.
         */
        /* convert to HSL */
        float *hsl;
        hsl = (float *) malloc(3 * size * sizeof(float));
        rgb2hsl(rgb, hsl, size);
        /* normalize the L channel */
        (void) balance_f32(hsl + 2 * size, size, nb_min, nb_max);
        /* convert back to RGB */
        hsl2rgb(hsl, rgb, size);
        free(hsl);
    }
    else if (0 == strcmp(argv[1], "hsv")) {
        /*
         * simplest color balance in the HSV color space, V channel
         *
         * The input image is normalized by affine transformation on
         * the V axis, saturating a percentage of the pixels at the
         * beginning and end of the axis.
         */
        /* convert to HSV */
        float *hsv;
        hsv = (float *) malloc(3 * size * sizeof(float));
        rgb2hsv(rgb, hsv, size);
        /* normalize the V channel */
        (void) balance_f32(hsv + 2 * size, size, nb_min, nb_max);
        /* convert back to RGB */
        hsv2rgb(hsv, rgb, size);
        free(hsv);
    }
    else if (0 == strcmp(argv[1], "hsi")) {
        /*
         * simplest color balance in the HSI color space, I channel
         *
         * The input image is normalized by affine transformation on
         * the I axis, saturating a percentage of the pixels at the
         * beginning and end of the axis. After converting back to the
         * RGB model, if the result is out of the RGB cube it will be
         * clipped.
         */
        /* convert to HSI */
        float *hsi;
        hsi = (float *) malloc(3 * size * sizeof(float));
        rgb2hsi(rgb, hsi, size);
        /* normalize the I channel */
        (void) balance_f32(hsi + 2 * size, size, nb_min, nb_max);
        /* convert back to RGB */
        hsi2rgb(hsi, rgb, size);
        free(hsi);
    }
    else {
        fprintf(stderr, "mode must be rgb, hsl, hsv or hsi\n");
        free(rgb);
        return EXIT_FAILURE;
    }

    /* write the PNG image and free the memory space */
    io_png_write_f32(argv[5], rgb, nx, ny, 3);
    free(rgb);

    return EXIT_SUCCESS;
}

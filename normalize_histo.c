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
 * @mainpage Simplest Color Balance
 *
 * README.txt:
 * @verbinclude README.txt
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
 * @author Jose-Luis Lisani <joseluis.lisani@uib.es>
 * @author Catalina Sbert <catalina.sbert@uib.es>
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
    float s1, s2;               /* saturated percentage */
    size_t nx, ny;              /* data size */
    unsigned char *data;        /* input/output data (each color channel processed) */
    unsigned char *data2;       /* input/output data (only intensity channel processed) */
    unsigned char *data3;       /*   data (only intensity channel processed) */
    unsigned char *I, *Inew;    /* intensity values */
    int channel, ii;
    unsigned char min, ming, maxg;

    size_t per_r0, per_g0, per_b0;
    float fI;
    size_t n;

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

    /* flattening percentage */
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

    data2 = malloc(3 * nx * ny * sizeof(unsigned char));
    memcpy(data2, data, 3 * nx * ny * sizeof(unsigned char));
    data3 = malloc(3 * nx * ny * sizeof(unsigned char));
    memcpy(data3, data, 3 * nx * ny * sizeof(unsigned char));
    /*
     * do normalization on RGB channels
     * we saturate s% pixels, half on both sides of the histogram
     */
    for (channel = 0; channel < 3; channel++) {
        (void) normalize_histo_u8(data + channel * nx * ny, nx * ny, 0, 255,
                                  nx * ny * (s1 / 100.),
                                  nx * ny * (s2 / 100.));

    }

    /*
     * do normalization on Intensity channel
     * we saturate s1% pixels, on left side and s2% pixels on the right side of  the histogram
     */
    /* Compute intensity values: I=(R+G+B)/3 */
    I = malloc(nx * ny * sizeof(unsigned char));
    Inew = malloc(nx * ny * sizeof(unsigned char));
    for (n = 0; n < (int) nx * ny; n++) {
        fI = ((float) data2[n] + (float) data2[n + nx * ny] +
              (float) data2[n + 2 * nx * ny]) / 3.;
        ii = (int) (fI + 0.5f);
        I[n] = (unsigned char) ii;
    }

    memcpy(Inew, I, nx * ny * sizeof(unsigned char));

    /*compute the numbers of pixels whose value is zero in each channel */
    per_r0 = 0;
    per_g0 = 0;
    per_b0 = 0;
    for (n = 0; n < (int) nx * ny; n++) {
        if (data2[n] == 0)
            per_r0++;
        if (data2[n + nx * ny] == 0)
            per_g0++;
        if (data2[n + 2 * nx * ny] == 0)
            per_b0++;
    }

    /* compute the minimum value of the gray level such that at most s1% of pixels sature to zero in one channel */

    if (per_r0 > nx * ny * (s1 / 100.) || per_g0 > nx * ny * (s1 / 100.)
        || per_b0 > nx * ny * (s1 / 100.))
        ming = 0;
    else {
        minmax_histo_u8(I, nx * ny, nx * ny * (s1 / 100.),
                        nx * ny * (s2 / 100.), &ming, &maxg);
        maxg = 255;
        do {
            (void) normalize_histo_u8_gray(Inew, nx * ny, 0, 255, ming, maxg);
            color_u8(data3, data2, I, Inew, nx * ny);
            per_r0 = 0;
            per_g0 = 0;
            per_b0 = 0;
            for (n = 0; n < (int) nx * ny; n++) {
                if (data3[n] == 0)
                    per_r0++;
                if (data3[n + nx * ny] == 0)
                    per_g0++;
                if (data3[n + 2 * nx * ny] == 0)
                    per_b0++;
            }

            if (per_r0 > nx * ny * (s1 / 100.)
                || per_g0 > nx * ny * (s1 / 100.)
                || per_b0 > nx * ny * (s1 / 100.))
                ming--;

            memcpy(Inew, I, nx * ny * sizeof(unsigned char));

        } while ((per_r0 > nx * ny * (s1 / 100.)
                  || per_g0 > nx * ny * (s1 / 100.)
                  || per_b0 > nx * ny * (s1 / 100.)));

    }

    /*compute the numbers of pixels whose value is 255 in each channel */
    per_r0 = 0;
    per_g0 = 0;
    per_b0 = 0;
    for (n = 0; n < (int) nx * ny; n++) {
        if (data2[n] == 255)
            per_r0++;
        if (data2[n + nx * ny] == 255)
            per_g0++;
        if (data2[n + 2 * nx * ny] == 255)
            per_b0++;
    }

    /* compute the maximum value of the gray level such that at most s2% of pixels sature to 255 in one channel */

    if (per_r0 > nx * ny * (s2 / 100.) || per_g0 > nx * ny * (s2 / 100.)
        || per_b0 > nx * ny * (s2 / 100.))
        maxg = 255;
    else {
        minmax_histo_u8(I, nx * ny, nx * ny * (s1 / 100.),
                        nx * ny * (s2 / 100.), &min, &maxg);

        do {
            /* normalize the gray intensity with the ming and maxg */
            (void) normalize_histo_u8_gray(Inew, nx * ny, 0, 255, ming, maxg);
            /* compute each channel for the new gray */

            color_u8(data3, data2, I, Inew, nx * ny);
            /*compute the numbers of pixels whose value is 255 in each channel */
            per_r0 = 0;
            per_g0 = 0;
            per_b0 = 0;
            for (n = 0; n < (int) nx * ny; n++) {
                if (data3[n] == 255)
                    per_r0++;
                if (data3[n + nx * ny] == 255)
                    per_g0++;
                if (data3[n + 2 * nx * ny] == 255)
                    per_b0++;
            }

            if (per_r0 > nx * ny * (s2 / 100.)
                || per_g0 > nx * ny * (s2 / 100.)
                || per_b0 > nx * ny * (s2 / 100.))
                maxg++;
            memcpy(Inew, I, nx * ny * sizeof(unsigned char));
        } while ((per_r0 > nx * ny * (s2 / 100.)
                  || per_g0 > nx * ny * (s2 / 100.)
                  || per_b0 > nx * ny * (s2 / 100.)));

    }

    /* write the PNG image */
    io_png_write_u8(argv[4], data, nx, ny, 3);
    io_png_write_u8(argv[5], data3, nx, ny, 3);

    free(data);
    free(data2);
    free(data3);
    free(I);
    free(Inew);

    return EXIT_SUCCESS;
}

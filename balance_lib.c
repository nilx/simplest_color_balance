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
 * @file balance_lib.c
 * @brief simple color balance routines
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 * @author Jose-Luis Lisani <joseluis.lisani@uib.es>
 * @author Catalina Sbert <catalina.sbert@uib.es>
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

/* ensure consistency */
#include "balance_lib.h"

/**
 * @brief get the min/max of an unsigned char array
 *
 * @param data input array
 * @param size array size
 * @param ptr_min, ptr_max pointers to the returned values, ignored if NULL
 */
static void minmax_u8(const unsigned char *data, size_t size,
                      unsigned char *ptr_min, unsigned char *ptr_max)
{
    unsigned char min, max;
    size_t i;

    /* sanity check */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }

    /* compute min and max */
    min = data[0];
    max = data[0];
    for (i = 1; i < size; i++) {
        if (data[i] < min)
            min = data[i];
        if (data[i] > max)
            max = data[i];
    }

    /* save min and max to the returned pointers if available */
    if (NULL != ptr_min)
        *ptr_min = min;
    if (NULL != ptr_max)
        *ptr_max = max;
    return;
}

/**
 * @brief get quantiles from an unsigned char array such that a given
 * number if pixels is out of this interval
 *
 * This function operates in-place. It computes min (resp. max) such
 * that the number of pixels < min (resp. > max) is inferior or equal
 * to nb_min (resp. nb_max).
 *
 * @param data input/output
 * @param size data array size
 * @param nb_min, nb_max number of pixels to flatten
 * @param ptr_min, ptr_max computed min/max output, ignored if NULL
 */
static void quantiles_u8(unsigned char *data, size_t size,
                         size_t nb_min, size_t nb_max,
                         unsigned char *ptr_min, unsigned char *ptr_max)
{
    /*
     * the histogran must hold all possible "unsigned char" values,
     * including 0
     */
    size_t h_size = UCHAR_MAX + 1;
    size_t histo[UCHAR_MAX + 1];
    size_t i;

    /* sanity check */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
    if (nb_min + nb_max >= size) {
        nb_min = (size - 1) / 2;
        nb_max = (size - 1) / 2;
        fprintf(stderr, "the number of pixels to flatten is too large\n");
        fprintf(stderr, "using (size - 1) / 2\n");
    }

    /* set the histogram to 0 */
    memset(histo, 0x00, h_size * sizeof(size_t));

    /* make an histogram */
    for (i = 0; i < size; i++)
        histo[(size_t) data[i]] += 1;

    /* convert the histogram to a cumulative histogram */
    for (i = 1; i < h_size; i++)
        histo[i] += histo[i - 1];

    /* get the new min/max */

    if (NULL != ptr_min) {
        /* simple forward traversal of the cumulative histogram */
        /* search the first value > nb_min */
        i = 0;
        while (i < h_size && histo[i] <= nb_min)
            i++;
        /* the corresponding histogram value is the current cell indice */
        *ptr_min = (unsigned char) i;
    }

    if (NULL != ptr_max) {
        /* simple backward traversal of the cumulative histogram */
        /* search the first value <= size - nb_max */
        i = h_size - 1;
        /* i is unsigned, we check i<h_size instead of i>=0 */
        while (i < h_size && histo[i] > (size - nb_max))
            i--;
        /*
         * if we are not at the end of the histogram,
         * get to the next cell,
         * ie the last (backward) value > size - nb_max
         */
        if (i < h_size - 1)
            i++;
        *ptr_max = (unsigned char) i;
    }

    return;
}

/**
 * @brief rescale an unsigned char array
 *
 * This function operates in-place. It rescales the data by a bounded
 * affine function such that min becomes 0 and max becomes UCHAR_MAX.
 *
 * @param data input/output array
 * @param size array size
 * @param min the minimum of the input array
 * @param nb_max number extremal pixels flattened
 *
 * @return data
 */
static unsigned char *rescale_u8(unsigned char *data, size_t size,
                                 unsigned char min, unsigned char max)
{
    size_t i;

    /* sanity checks */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }

    /* printf("min=%d  max=%d\n", min, max); */
    if (max <= min) {
        unsigned char mid = UCHAR_MAX / 2;
        for (i = 0; i < size; i++)
            data[i] = mid;
    }
    else {
        /* build a normalization table */
        unsigned char norm[UCHAR_MAX + 1];
        for (i = 0; i < min; i++)
            norm[i] = 0;
        for (i = max; i < UCHAR_MAX + 1; i++)
            norm[i] = UCHAR_MAX;
        for (i = min; i < max; i++)
            /*
             * we can't store and reuse UCHAR_MAX / (max - min) because
             *     105 * 255 / 126.            -> 212.5, rounded to 213
             *     105 * (double) (255 / 126.) -> 212.4999, rounded to 212
             */
            norm[i] = (unsigned char) floor((i - min) * UCHAR_MAX
                                            / (float) (max - min) + .5);
        /* use the normalization table to transform the data */
        for (i = 0; i < size; i++)
            data[i] = norm[(size_t) data[i]];
    }
    return data;
}

/**
 * @brief normalize an unsigned char array
 *
 * This function operates in-place. It computes the minimum and
 * maximum values of the data, and rescales the data to
 * [0-UCHAR_MAX], with optionnally flattening some extremal pixels.
 *
 * @param data input/output array
 * @param size array size
 * @param nb_min, nb_max number extremal pixels flattened
 *
 * @return data
 */
unsigned char *balance_u8(unsigned char *data, size_t size,
                          size_t nb_min, size_t nb_max)
{
    unsigned char min, max;

    /* sanity checks */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
    if (nb_min + nb_max >= size) {
        nb_min = (size - 1) / 2;
        nb_max = (size - 1) / 2;
        fprintf(stderr, "the number of pixels to flatten is too large\n");
        fprintf(stderr, "using (size - 1) / 2\n");
    }

    if (0 != nb_min || 0 != nb_max)
        /* get the min/max from the histogram */
        quantiles_u8(data, size, nb_min, nb_max, &min, &max);
    else
        /* get the min/max from the data */
        minmax_u8(data, size, &min, &max);

    /* rescale */
    (void) rescale_u8(data, size, min, max);

    return data;
}

/**
 * @brief computes the R G B components of the output image from its
 * gray level
 *
 * Given a color image C=(R, G, B) and its intensity I=(R+G+B)/3 and a
 * modified intensity I', this function computes an output color image
 * C'=(R', G', B') where each channel is proportional to the input
 * channel and whose intensity is I': R'=I'/I R, G'=I'/I G, B'=I'/I B
 *
 * If the factor I'/I is too large the colors may be saturated to the
 * faces of the RGB cube.
 *
 * @todo fix algo bug, the output image intensity is not I'
 *
 * @param data_out output color image
 * @param data_in input color image
 * @param gray gray level of the input color image
 * @param gray1 modified gray image
 * @param dim size of the image
 *
 * @return data_out
 */
static void color_u8(unsigned char *data_out, unsigned char *data_in,
                     unsigned char *gray_in, unsigned char *gray_out,
                     size_t size)
{
    unsigned char *r_in, *g_in, *b_in;
    unsigned char *r_out, *g_out, *b_out;
    float scale, r, g, b;
    size_t i;

    /* sanity check */
    if (NULL == data_out || NULL == data_in
        || NULL == gray_out || NULL == gray_in) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }

    /* setup the direct RGB channel pointers */
    r_out = data_out;
    g_out = data_out + size;
    b_out = data_out + 2 * size;
    r_in = data_in;
    g_in = data_in + size;
    b_in = data_in + 2 * size;

    for (i = 0; i < size; i++) {
        scale = (0 == gray_in[i] ? 0.
                 : (float) gray_out[i] / (float) gray_in[i]);
        r = scale * (float) r_in[i];
        g = scale * (float) g_in[i];
        b = scale * (float) b_in[i];
        /* if this results in RGB > UCHAR_MAX, use a smaller scaling factor */
        if (r > (float) UCHAR_MAX
            || g > (float) UCHAR_MAX || b > (float) UCHAR_MAX) {
            scale = (float) r_in[i];
            if (scale < (float) g_in[i])
                scale = (float) g_in[i];
            if (scale < (float) b_in[i])
                scale = (float) b_in[i];
            scale = UCHAR_MAX / scale;
            r = scale * (float) r_in[i];
            g = scale * (float) g_in[i];
            b = scale * (float) b_in[i];
        }
        /* save, rounded */
        r_out[i] = (unsigned char) floor(r + .5);
        g_out[i] = (unsigned char) floor(g + .5);
        b_out[i] = (unsigned char) floor(b + .5);
    }
    return;
}

/**
 * @brief convert a RGB array to its intensity
 *
 * @param data_rgb input array
 * @param data_i output array
 * @param size array size
 *
 * @return data_i
 */
static unsigned char *rgb2i_u8(const unsigned char *data_rgb,
                               unsigned char *data_i, size_t size)
{
    const unsigned char *data_r, *data_g, *data_b;
    size_t i;

    data_r = data_rgb;
    data_g = data_rgb + size;
    data_b = data_rgb + 2 * size;
    for (i = 0; i < size; i++)
        data_i[i] = (unsigned char)
            floor(((float) data_r[i]
                   + (float) data_g[i]
                   + (float) data_b[i]) / 3. + .5);
    return data_i;
}

/**
 * @brief count occurences in an array
 *
 * @param data input array
 * @param size array size
 * @param value counted occurence
 *
 * @return number of times value appears in data
 */
static size_t count_u8(const unsigned char *data,
                       size_t size, unsigned char value)
{
    size_t i, nb;

    nb = 0;
    for (i = 0; i < size; i++)
        nb += (value == data[i]);
    return nb;
}

/**
 * @brief normalize a RGB unsigned char array via its intensity
 *
 * This function operates in-place. It computes the minimum and
 * maximum values of the data, and rescales the data to
 * [0-UCHAR_MAX], with optionnally flattening some extremal pixels.
 *
 * @param data input/output array
 * @param size array size
 * @param nb_min, nb_max number extremal pixels flattened
 *
 * @return data
 */
unsigned char *balance_intensity_u8(unsigned char *data, size_t size,
                                    size_t nb_min, size_t nb_max)
{
    unsigned char *data_out;    /* data (intensity process) */
    unsigned char *intensity, *intensity_out;   /* intensity values */
    unsigned char min, max;
    size_t nb_r, nb_g, nb_b;

    data_out = (unsigned char *) malloc(3 * size * sizeof(unsigned char));
    memcpy(data_out, data, 3 * size * sizeof(unsigned char));
    intensity = (unsigned char *) malloc(size * sizeof(unsigned char));
    intensity_out = (unsigned char *) malloc(size * sizeof(unsigned char));

    /* compute intensity values: I=(R+G+B)/3 */
    (void) rgb2i_u8(data, intensity, size);
    memcpy(intensity_out, intensity, size * sizeof(unsigned char));

    /*
     * compute the numbers of pixels
     * whose value is zero in each channel
     */
    nb_r = count_u8(data, size, 0);
    nb_g = count_u8(data + size, size, 0);
    nb_b = count_u8(data + 2 * size, size, 0);

    /*
     * compute the minimum value of the intensity
     * such that at most nb_min of pixels sature to zero in one channel
     */
    if (nb_r > nb_min || nb_g > nb_min || nb_b > nb_min)
        min = 0;
    else {
        quantiles_u8(intensity, size, nb_min, nb_max, &min, NULL);
        max = UCHAR_MAX;
        do {
            /* normalize the gray intensity with the min and max */
            (void) rescale_u8(intensity_out, size, min, max);
            /* compute each channel for the new gray */
            color_u8(data_out, data, intensity, intensity_out, size);
            /*
             * recompute the numbers of pixels
             * whose value is 0 in each channel
             */
            nb_r = count_u8(data_out, size, 0);
            nb_g = count_u8(data_out + size, size, 0);
            nb_b = count_u8(data_out + 2 * size, size, 0);

            if (nb_r > nb_min || nb_g > nb_min || nb_b > nb_min)
                min--;
            memcpy(intensity_out, intensity, size * sizeof(unsigned char));
        } while (nb_r > nb_min || nb_g > nb_min || nb_b > nb_min);
    }

    /*
     * compute the numbers of pixels
     * whose value is UCHAR_MAX in each channel
     */
    nb_r = count_u8(data, size, UCHAR_MAX);
    nb_g = count_u8(data + size, size, UCHAR_MAX);
    nb_b = count_u8(data + 2 * size, size, UCHAR_MAX);

    /*
     * compute the maximum value of the intensity
     * such that at most nb_max pixels sature to UCHAR_MAX in one channel
     */
    if (nb_r > nb_max || nb_g > nb_max || nb_b > nb_max)
        max = UCHAR_MAX;
    else {
        quantiles_u8(intensity, size, nb_min, nb_max, NULL, &max);
        do {
            /* normalize the gray intensity with the min and max */
            (void) rescale_u8(intensity_out, size, min, max);
            /* compute each channel for the new gray */
            color_u8(data_out, data, intensity, intensity_out, size);
            /*
             * recompute the numbers of pixels
             * whose value is UCHAR_MAX in each channel
             */
            nb_r = count_u8(data_out, size, UCHAR_MAX);
            nb_g = count_u8(data_out + size, size, UCHAR_MAX);
            nb_b = count_u8(data_out + 2 * size, size, UCHAR_MAX);

            if (nb_r > nb_max || nb_g > nb_max || nb_b > nb_max)
                max++;
            memcpy(intensity_out, intensity, size * sizeof(unsigned char));
        } while (nb_r > nb_max || nb_g > nb_max || nb_b > nb_max);
    }

    free(intensity);
    free(intensity_out);

    memcpy(data, data_out, 3 * size * sizeof(unsigned char));
    free(data_out);

    return data;
}

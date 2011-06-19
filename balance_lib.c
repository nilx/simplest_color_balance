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

/* encure consistency */
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
void quantiles_u8(unsigned char *data, size_t size,
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
        /* search the first value > flat_nb_min */
        i = 0;
        while (i < h_size && histo[i] <= nb_min)
            i++;
        /* the corresponding histogram value is the current cell indice */
        if (i > 0)
            i--;
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
         * ie the last (backward) value > size - flat_nb_max
         */
        if (i < h_size - 1)
            i++;
        *ptr_max = (unsigned char) i;
    }

    return;
}

/**
 * @brief normalize an unsigned char array
 *
 * This function operates in-place. It computes the minimum and
 * maximum values of the data, and rescales the data to the target
 * minimum and maximum, with optionnally flattening some extremal
 * pixels.
 *
 * @param data input/output array
 * @param size array size
 * @param target_min, target_max target min/max values
 * @param flat_nb_min, flat_nb_max number extremal pixels flattened
 *
 * @return data
 *
 * @todo hardcode target_*
 */
unsigned char *balance_u8(unsigned char *data, size_t size,
                          unsigned char target_min,
                          unsigned char target_max,
                          size_t flat_nb_min, size_t flat_nb_max)
{
    unsigned char min, max;
    size_t i;

    /* sanity checks */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }
    if (flat_nb_min + flat_nb_max >= size) {
        flat_nb_min = (size - 1) / 2;
        flat_nb_max = (size - 1) / 2;
        fprintf(stderr, "the number of pixels to flatten is too large\n");
        fprintf(stderr, "using (size - 1) / 2\n");
    }

    /* target_max == target_min : shortcut */
    if (target_max == target_min) {
        for (i = 0; i < size; i++)
            data[i] = target_min;
        return data;
    }

    if (0 != flat_nb_min || 0 != flat_nb_max)
        /* get the min/max from the histogram */
        quantiles_u8(data, size, flat_nb_min, flat_nb_max, &min, &max);
    else
        /* get the min/max from the data */
        minmax_u8(data, size, &min, &max);

    /* rescale */
    /* max <= min : constant output */
    if (max <= min) {
        unsigned char target_mid;
        target_mid = (target_max + target_min) / 2;
        for (i = 0; i < size; i++)
            data[i] = target_mid;
    }
    else {
        /*
         * build a bounded linear normalization table
         * such that norm(min) = target_min
         *           norm(max) = target_max
         * norm(x) = (x - min) * (t_max - t_min) / (max - min) + t_min
         */
        float scale;
        unsigned char norm[UCHAR_MAX + 1];

        scale = (float) (target_max - target_min) / (float) (max - min);
        for (i = 0; i < min; i++)
            norm[i] = target_min;
        for (i = min; i < max; i++)
            norm[i] = (unsigned char) ((i - min) * scale + target_min);
        for (i = max; i < UCHAR_MAX + 1; i++)
            norm[i] = target_max;
        /* use the normalization table to transform the data */
        for (i = 0; i < size; i++)
            data[i] = norm[(size_t) data[i]];
    }
    return data;
}

/**
 * @brief normalize an unsigned char array
 *
 * This function operates in-place. It rescales the data by a bounded
 * affine function such that min becomes target_min and max becomes
 * target_max.
 *
 * @param data input/output array
 * @param size array size
 * @param target_min, target_max target min/max values
 * @param min the minimum of the input array
 * @param flat_nb_max number extremal pixels flattened
 *
 * @return data
 *
 * @todo hardcode target_*
 */

unsigned char *balance_u8_gray(unsigned char *data, size_t size,
                               unsigned char target_min,
                               unsigned char target_max,
                               unsigned char min, unsigned char max)
{
    size_t i;

    /* sanity checks */
    if (NULL == data) {
        fprintf(stderr, "a pointer is NULL and should not be so\n");
        abort();
    }

    /* target_max == target_min : shortcut */
    if (target_max == target_min) {
        for (i = 0; i < size; i++)
            data[i] = target_min;
        return data;
    }

    /* printf("min=%d  max=%d\n", min, max); */
    if (max <= min) {
        unsigned char target_mid;
        target_mid = (target_max + target_min) / 2;
        for (i = 0; i < size; i++)
            data[i] = target_mid;
    }
    else {
        float scale;
        unsigned char norm[UCHAR_MAX + 1];

        scale = (float) (target_max - target_min) / (float) (max - min);
        for (i = 0; i < min; i++)
            norm[i] = target_min;
        for (i = min; i < max; i++)
            norm[i] = (unsigned char) ((i - min) * scale + target_min);
        for (i = max; i < UCHAR_MAX + 1; i++)
            norm[i] = target_max;
        /* use the normalization table to transform the data */
        for (i = 0; i < size; i++)
            data[i] = norm[(size_t) data[i]];
    }

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

void color_u8(unsigned char *data_out, unsigned char *data_in,
              unsigned char *gray_in, unsigned char *gray_out, size_t size)
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
        /* if this results in RGB > 255, use a smaller scaling factor */
        if (r > 255. || g > 255. || b > 255.) {
            scale = (float) r_in[i];
            if (scale < (float) g_in[i])
                scale = (float) g_in[i];
            if (scale < (float) b_in[i])
                scale = (float) b_in[i];
            scale = 255. / scale;
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

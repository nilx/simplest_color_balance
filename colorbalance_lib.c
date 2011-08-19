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
 * @file colorbalance_lib.c
 * @brief color balance algorithms
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 * @author Jose-Luis Lisani <joseluis.lisani@uib.es>
 * @author Catalina Sbert <catalina.sbert@uib.es>
 *
 * @todo check precision loss, maybe use double precision values
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include "balance_lib.h"
#include "colorspace_lib.h"

/* ensure consistency */
#include "colorbalance_lib.h"

/**
 * @brief simplest color balance on RGB channels
 *
 * The input image is normalized by affine transformation on each RGB
 * channel, saturating a percentage of the pixels at the beginning and
 * end of the color space on each channel.
 *
 * @todo integer and histogram-based alternative
 */
float *colorbalance_rgb_f32(float *rgb, size_t size,
                            size_t nb_min, size_t nb_max)
{
    (void) balance_f32(rgb, size, nb_min, nb_max);
    (void) balance_f32(rgb + size, size, nb_min, nb_max);
    (void) balance_f32(rgb + 2 * size, size, nb_min, nb_max);
    return rgb;
}

/**
 * @brief simplest color balance in the HSL color space, L axis
 *
 * The input image is normalized by affine transformation on the L
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis.
 *
 * @todo compute L from RGB, then compute new RGB from RGB and new L
 */
float *colorbalance_hsl_f32(float *rgb, size_t size,
                            size_t nb_min, size_t nb_max)
{
    float *hsl;
    /* convert to HSL */
    hsl = (float *) malloc(3 * size * sizeof(float));
    rgb2hsl(rgb, hsl, size);
    /* normalize the L channel */
    (void) balance_f32(hsl + 2 * size, size, nb_min, nb_max);
    /* convert back to RGB */
    hsl2rgb(hsl, rgb, size);
    free(hsl);
    return rgb;
}

/**
 * @brief simplest color balance in the HSV color space, V axis
 *
 * The input image is normalized by affine transformation on the V
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis.
 *
 * @todo compute V from RGB, then compute new RGB from RGB and new V
 */
float *colorbalance_hsv_f32(float *rgb, size_t size,
                            size_t nb_min, size_t nb_max)
{
    float *hsv;
    /* convert to HSV */
    hsv = (float *) malloc(3 * size * sizeof(float));
    rgb2hsv(rgb, hsv, size);
    /* normalize the V channel */
    (void) balance_f32(hsv + 2 * size, size, nb_min, nb_max);
    /* convert back to RGB */
    hsv2rgb(hsv, rgb, size);
    free(hsv);
    return rgb;
}

/**
 * @brief simplest color balance based on the I axis applied to the
 * RGB channels, bounded
 *
 * The input image is normalized by affine transformation on the I
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis. This transformation is linearly applied to the R,
 * G and B channels. The RGB cube is not stable by this operation, so
 * some clipping will happen when the result is out of the RGB cube.
 */
float *colorbalance_irgb_bounded_f32(float *rgb, size_t size,
                                     size_t nb_min, size_t nb_max)
{
    float *irgb, *inorm;        /* intensity scale */
    double s;
    size_t i;
    /** @todo compute I=R+G+B instead of (R+G+B)/3 to save a division */
    irgb = (float *) malloc(size * sizeof(float));
    for (i = 0; i < size; i++)
        irgb[i] = (rgb[i] + rgb[i + size] + rgb[i + 2 * size]) / 3.;
    /* copy and normalize I */
    inorm = (float *) malloc(size * sizeof(float));
    memcpy(inorm, irgb, size * sizeof(float));
    (void) balance_f32(inorm, size, nb_min, nb_max);
    /*
     * apply the I normalization to the RGB channels:
     * RGB = RGB * Inorm / I
     */
    for (i = 0; i < size; i++) {
        s = inorm[i] / irgb[i];
        rgb[i] *= s;
        rgb[i + size] *= s;
        rgb[i + 2 * size] *= s;
    }
    /* clip RGB values to [0, 1]; overflow can only happen on 1 */
    for (i = 0; i < 3 * size; i++)
        rgb[i] = (rgb[i] > 1. ? 1. : rgb[i]);
    free(irgb);
    free(inorm);
    return rgb;
}

/** @brief min of A and B */
#define MIN(A,B) (((A) <= (B)) ? (A) : (B))

/** @brief max of A and B */
#define MAX(A,B) (((A) >= (B)) ? (A) : (B))

/** @brief max of A, B, and C */
#define MAX3(A,B,C) (((A) >= (B)) ? MAX(A,C) : MAX(B,C))

/**
 * @brief double comparison
 *
 * @todo use integer ops, cf cmp_f32
 */
static int cmp_f64(const void *a, const void *b)
{
    if (*(const double *) a > *(const double *) b)
        return 1;
    if (*(const double *) a < *(const double *) b)
        return -1;
    return 0;
}

/**
 * @brief simplest color balance based on the I axis applied to the
 * RGB channels, adjusted
 *
 * The input image is normalized by affine transformation on the I
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis. This transformation is linearly applied to the R,
 * G and B channels. The RGB cube is not stable by this operation, so
 * to avoid clipping the linear scaling factors are adjusted to
 * maintain the R/G/B ratios.
 *
 * @todo explain I / (max(R, G, B) * (I - Imin))
 */
float *colorbalance_irgb_adjusted_f32(float *rgb, size_t size,
                                      size_t nb_min, size_t nb_max)
{
    float *maxrgb;
    double *irgb, *tmp;
    double imin, alpha, beta, s;
    size_t i;

    /* compute I=(R+G+B)/3 */
    /** @todo work with I=R+G+B instead of (R+G+B)/3 to save a division */
    irgb = (double *) malloc(size * sizeof(double));
    tmp = (double *) malloc(size * sizeof(double));
    for (i = 0; i < size; i++)
        irgb[i] = (rgb[i] + rgb[i + size] + rgb[i + 2 * size]) / 3;
    /*
     * sort I and get Imin, the nb_min-th I value
     * to be mapped to 0 in order to saturate nb_min pixels
     */
    memcpy(tmp, irgb, size * sizeof(double));
    qsort(tmp, size, sizeof(double), &cmp_f64);
    imin = tmp[nb_min];

    /* compute  I / (max(R, G, B) * (I - Imin)) */
    maxrgb = (float *) malloc(size * sizeof(float));
    for (i = 0; i < size; i++)
        maxrgb[i] = MAX3(rgb[i], rgb[i + size], rgb[i + 2 * size]);
    for (i = 0; i < size; i++)
        tmp[i] = maxrgb[i] * (irgb[i] - imin);
    /* avoid divisions by 0 */
    for (i = 0; i < size; i++)
        tmp[i] = (0 == tmp[i] ? -FLT_MIN : irgb[i] / tmp[i]);
    /*
     * sort and get alpha, the nb_max-th positive value
     * to be used as the scaling factor to saturate nb_max pixels
     */
    qsort(tmp, size, sizeof(double), &cmp_f64);
    i = 0;
    while ((i < size) && (tmp[i] <= 0))
        i++;
    /* avoid out-of-bounds errors */
    i = MIN(i + nb_max, size - 1);
    alpha = tmp[i];
    beta = -alpha * imin;
    free(tmp);

    /* normalize I */
    for (i = 0; i < size; i++) {
        /* if I = 0, it will be mapped to a value <= 0 */
        s = (0. == irgb[i] ? 0. : (alpha * irgb[i] + beta) / irgb[i]);
        /*
         * the scaling factor is adjusted
         * such that R, G and B keep under 1 and are not truncated
         */
        s = MAX(s, 0.);
        s = MIN(s, 1. / maxrgb[i]);
        rgb[i] *= s;
        rgb[i + size] *= s;
        rgb[i + 2 * size] *= s;
    }
    free(irgb);
    free(maxrgb);

    return rgb;
}

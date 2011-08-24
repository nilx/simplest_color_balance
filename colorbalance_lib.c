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

/* ensure consistency */
#include "colorbalance_lib.h"

/**
 * @brief simplest color balance on RGB channels
 *
 * The input image is normalized by affine transformation on each RGB
 * channel, saturating a percentage of the pixels at the beginning and
 * end of the color space on each channel.
 */
unsigned char *colorbalance_rgb_u8(unsigned char *rgb, size_t size,
                                   size_t nb_min, size_t nb_max)
{
    (void) balance_u8(rgb, size, nb_min, nb_max);
    (void) balance_u8(rgb + size, size, nb_min, nb_max);
    (void) balance_u8(rgb + 2 * size, size, nb_min, nb_max);
    return rgb;
}

/** @brief max of A and B */
#define MAX(A,B) (((A) >= (B)) ? (A) : (B))

/** @brief max of A, B, and C */
#define MAX3(A,B,C) (((A) >= (B)) ? MAX(A,C) : MAX(B,C))

/**
 * @brief simplest color balance based on the I axis applied to the
 * RGB channels, bounded
 *
 * The input image is normalized by affine transformation on the I
 * axis, saturating a percentage of the pixels at the beginning and
 * end of the axis. This transformation is linearly applied to the R,
 * G and B channels. The RGB cube is not stable by this operation, so
 * some projections towards (0,0,0) on the RGB cube will be performed
 * if needed.
 */
float *colorbalance_irgb_f32(float *rgb, size_t size,
                             size_t nb_min, size_t nb_max)
{
    float *irgb, *inorm;        /* intensity scale */
    double s, m;
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
     * RGB = RGB * Inorm / I, with a projection towards (0,0,0)
     * on the RGB cube if needed
     */
    for (i = 0; i < size; i++) {
        m = MAX3(rgb[i], rgb[i + size], rgb[i + 2 * size]);
        s = inorm[i] / irgb[i];
        /* if m * s > 1, a projection is needed by adjusting s */
        s = (1. < m * s ? 1. / m : s);
        rgb[i] *= s;
        rgb[i + size] *= s;
        rgb[i + 2 * size] *= s;
    }
    free(irgb);
    free(inorm);
    return rgb;
}

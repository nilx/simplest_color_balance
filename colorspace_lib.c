/*
 * Copyright 2005-2010, Pascal Getreuer <pascal.getreuer@cmla.ens-cachan.fr>
 * Copyright 2011, Niclas Limarer <nicolas.limare@cmla.ens-cachan.fr>
 * All rights reserved.
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
 * @file colorspace_lib.c
 * @brief color space conversion routines

 * The conversion code is derived from Pascal Getreuer's colorspace
 * conversion library; he explicitely allowed the relicensing of the
 * RGB/HSL functions from BSD to GPL.
 * http://www.math.ucla.edu/~getreuer/colorspace.html
 *
 * @author Pascal Getreuer <pascal.getreuer@cmla.ens-cachan.fr>
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * @todo convert in-loop, no function call
 * @todo probably lots of room for speed improvements
 * @todo only compute L from RGB, then compute RGB from RGB and L
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

/* ensure consistency */
#include "colorspace_lib.h"

/** @brief min of A and B */
#define MIN(A,B) (((A) <= (B)) ? (A) : (B))

/** @brief max of A and B */
#define MAX(A,B) (((A) >= (B)) ? (A) : (B))

/** @brief min of A, B, and C */
#define MIN3(A,B,C) (((A) <= (B)) ? MIN(A,C) : MIN(B,C))

/** @brief max of A, B, and C */
#define MAX3(A,B,C) (((A) >= (B)) ? MAX(A,C) : MAX(B,C))

#ifndef M_PI
/** @brief The constant Pi */
#define M_PI 3.14159265358979323846264338327950288
#endif

/**
 * @brief Convert an sRGB color to Hue-Saturation-Lightness (HSL).
 *
 * This routine transforms from sRGB to the double hexcone HSL color
 * space. The sRGB values are assumed to be between 0 and 1.  The
 * outputs are
 *   H = hexagonal hue angle                (0 <= H < 6),
 *   S = { C/(2L)     if L <= 1/2           (0 <= S <= 1),
 *       { C/(2 - 2L) if L >  1/2
 *   L = (max(R',G',B') + min(R',G',B'))/2  (0 <= L <= 1),
 * where C = max(R',G',B') - min(R',G',B').  The inverse color
 * transformation is given by _hsl2rgb.
 *
 * @param r, g, b the input sRGB values scaled in [0,1]
 * @param h, s, l pointers to hold the HSL result
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _rgb2hsl(float r, float g, float b, float *h, float *s, float *l)
{
    float max, min, c;

    max = MAX3(r, g, b);
    min = MIN3(r, g, b);
    c = max - min;

    *l = (max + min) / 2;
    if (c > 0) {
        if (max == r) {
            *h = (g - b) / c;
            if (g < b)
                *h += 6;
        }
        else if (max == g)
            *h = 2 + (b - r) / c;
        else
            *h = 4 + (r - g) / c;
        *s = (*l <= 0.5 ? c / (2 * (*l))
              : c / (2 - 2 * (*l)));
    }
    else {
        *h = 0;
        *s = 0;
    }
}

/**
 * @brief Convert an array from sRGB to Hue-Saturation-Lightness (HSL).
 *
 * This routine uses _rgb2hsl() on a float array with sRGB values
 * in [0-UCHAR_MAX] to produce the equivalent float array
 * with HSL values in [0-6[x[0-1]x[0-1].
 *
 * @param rgb input array
 * @param hsl output array
 * @param size array size
 */
void rgb2hsl(const float *rgb, float *hsl, size_t size)
{
    const float *r, *g, *b;
    float *h, *s, *l;
    size_t i;

    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;
    h = hsl;
    s = hsl + size;
    l = hsl + 2 * size;

    for (i = 0; i < size; i++)
        _rgb2hsl(r[i] / UCHAR_MAX, g[i] / UCHAR_MAX, b[i] / UCHAR_MAX,
                 h + i, s + i, l + i);
}

/**
 * @brief Convert a Hue-Saturation-Lightness (HSL) color to sRGB.
 *
 * The input values are assumed to be scaled as
 *    0 <= H < 6,
 *    0 <= S <= 1,
 *    0 <= L <= 1.
 * The output sRGB values are scaled between 0 and 1.  This is the
 * inverse transformation of _rgb2hsl.
 *
 * @param h, s, l the input HSL values
 * @param r, g, b pointers to hold the sRGB result
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _hsl2rgb(float h, float s, float l, float *r, float *g, float *b)
{
    float c, min, x;

    c = (l <= 0.5 ? 2 * l * s : (2 - 2 * l) * s);
    min = l - 0.5 * c;
    x = c * (1 - fabs(h - 2 * floor(h / 2) - 1));

    switch ((int) h) {
    case 0:
        *r = min + c;
        *g = min + x;
        *b = min;
        break;
    case 1:
        *r = min + x;
        *g = min + c;
        *b = min;
        break;
    case 2:
        *r = min;
        *g = min + c;
        *b = min + x;
        break;
    case 3:
        *r = min;
        *g = min + x;
        *b = min + c;
        break;
    case 4:
        *r = min + x;
        *g = min;
        *b = min + c;
        break;
    case 5:
        *r = min + c;
        *g = min;
        *b = min + x;
        break;
    default:
        *r = 0;
        *g = 0;
        *b = 0;
    }
}

/**
 * @brief Convert an array from Hue-Saturation-Lightness (HSL) to sRGB.
 *
 * This routine uses _hsl2rgb() on a float array with HSL values in
 * [0-6[x[0-1]x[0-1] to produce the equivalent float array with sRGB
 * values in [0-UCHAR_MAX].
 *
 * @param hsl input array
 * @param rgb output array
 * @param size array size
 */
void hsl2rgb(const float *hsl, float *rgb, size_t size)
{
    const float *h, *s, *l;
    float *r, *g, *b;
    size_t i;

    h = hsl;
    s = hsl + size;
    l = hsl + 2 * size;
    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;

    for (i = 0; i < size; i++) {
        _hsl2rgb(h[i], s[i], l[i], r + i, g + i, b + i);
        r[i] *= UCHAR_MAX;
        g[i] *= UCHAR_MAX;
        b[i] *= UCHAR_MAX;
    }
}

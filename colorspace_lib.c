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
 * RGB/HSL/HSV/HSI functions from BSD to GPL.
 * http://www.math.ucla.edu/~getreuer/colorspace.html
 *
 * @author Pascal Getreuer <pascal.getreuer@cmla.ens-cachan.fr>
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * @todo convert in-loop, no function call
 * @todo probably lots of room for speed improvements
 * @todo only compute L from RGB, then compute RGB from RGB and new L
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

/*
 * SCALAR FUNCTIONS
 */

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
 * @brief Convert an sRGB color to Hue-Saturation-Value (HSV)
 *
 * This routine transforms from sRGB to the hexcone HSV color space.
 * The sRGB values are assumed to be between 0 and 1.  The output values
 * are
 *   H = hexagonal hue angle   (0 <= H < 6),
 *   S = C/V                   (0 <= S <= 1),
 *   V = max(R',G',B')         (0 <= V <= 1),
 * where C = max(R',G',B') - min(R',G',B').  The inverse color
 * transformation is given by _sv2rgb.
 *
 * @param r, g, b the input sRGB values scaled in [0,1]
 * @param h, s, v pointers to hold the HSV result
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _rgb2hsv(float r, float g, float b, float *h, float *s, float *v)
{
    float max, min, c;

    max = MAX3(r, g, b);
    min = MIN3(r, g, b);
    c = max - min;
    *v = max;

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

        *s = c / max;
    }
    else {
        *h = 0;
        *s = 0;
    }
}

/**
 * @brief Convert a Hue-Saturation-Value (HSV) color to sRGB
 *
 * The input values are assumed to be scaled as
 *    0 <= H < 6,
 *    0 <= S <= 1,
 *    0 <= V <= 1.
 * The output sRGB values are scaled between 0 and 1.  This is the
 * inverse transformation of _rgb2hsv.
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _hsv2rgb(float h, float s, float v, float *r, float *g, float *b)
{
    float c, min, x;

    c = s * v;
    min = v - c;
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
 * @brief Convert an sRGB color to Hue-Saturation-Intensity (HSI)
 *
 * This routine transforms from sRGB to the cylindrical HSI color
 * space.  The sRGB values are assumed to be between 0 and 1.  The
 * output values are
 *   H = polar hue angle         (0 <= H < 6),
 *   S = 1 - min(R',G',B')/I     (0 <= S <= 1),
 *   I = (R'+G'+B')/3            (0 <= I <= 1).
 * The inverse color transformation is given by _hsi2rgb.
 *
 * @param r, g, b the input sRGB values scaled in [0,1]
 * @param h, s, i pointers to hold the HSI result
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _rgb2hsi(float r, float g, float b, float *h, float *s, float *i)
{
    float alpha, beta;

    alpha = 0.5 * (2 * r - g - b);
    beta = 0.866025403784439 * (g - b);

    *i = (r + g + b) / 3;
    if (*i > 0.) {
        *s = 1 - MIN3(r, g, b) / *i;
        *h = atan2(beta, alpha) * (3 / M_PI);
        if (*h < 0)
            *h += 360;
    }
    else {
        *h = 0;
        *s = 0;
    }
}

/**
 * @brief Convert a Hue-Saturation-Intesity (HSI) color to sRGB
 *
 * @param R, G, B pointers to hold the result
 * @param H, S, I the input HSI values
 *
 * The input values are assumed to be scaled as
 *    0 <= H < 6,
 *    0 <= S <= 1,
 *    0 <= I <= 1.
 * The output sRGB values are scaled between 0 and 1.  This is the
 * inverse transformation of _rgb2hsi.
 *
 * @param h, s, i the input HSI values
 * @param r, g, b pointers to hold the sRGB result
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void _hsi2rgb(float h, float s, float i, float *r, float *g, float *b)
{
    if (h < 2) {
        *b = i * (1 - s);
        *r = i * (1 + s * cos(h * (M_PI / 3)) / cos((1 - h) * (M_PI / 3)));
        *g = 3 * i - *r - *b;
    }
    else if (h < 4) {
        h -= 2;
        *r = i * (1 - s);
        *g = i * (1 + s * cos(h * (M_PI / 3)) / cos((1 - h) * (M_PI / 3)));
        *b = 3 * i - *r - *g;
    }
    else {
        h -= 4;
        *g = i * (1 - s);
        *b = i * (1 + s * cos(h * (M_PI / 3)) / cos((1 - h) * (M_PI / 3)));
        *r = 3 * i - *g - *b;
    }
}

/*
 * VECTOR FUNCTIONS
 */

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
        _rgb2hsl(r[i], g[i], b[i], h + i, s + i, l + i);
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

    for (i = 0; i < size; i++)
        _hsl2rgb(h[i], s[i], l[i], r + i, g + i, b + i);
}

/**
 * @brief Convert an array from sRGB to Hue-Saturation-Value (HSV).
 *
 * This routine uses _rgb2hsv() on a float array with sRGB values
 * in [0-UCHAR_MAX] to produce the equivalent float array
 * with HSV values in [0-6[x[0-1]x[0-1].
 *
 * @param rgb input array
 * @param hsv output array
 * @param size array size
 */
void rgb2hsv(const float *rgb, float *hsv, size_t size)
{
    const float *r, *g, *b;
    float *h, *s, *v;
    size_t i;

    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;
    h = hsv;
    s = hsv + size;
    v = hsv + 2 * size;

    for (i = 0; i < size; i++)
        _rgb2hsv(r[i], g[i], b[i], h + i, s + i, v + i);
}

/**
 * @brief Convert an array from Hue-Saturation-Value (HSV) to sRGB.
 *
 * This routine uses _hsv2rgb() on a float array with HSV values in
 * [0-6[x[0-1]x[0-1] to produce the equivalent float array with sRGB
 * values in [0-UCHAR_MAX].
 *
 * @param hsv input array
 * @param rgb output array
 * @param size array size
 */
void hsv2rgb(const float *hsv, float *rgb, size_t size)
{
    const float *h, *s, *v;
    float *r, *g, *b;
    size_t i;

    h = hsv;
    s = hsv + size;
    v = hsv + 2 * size;
    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;

    for (i = 0; i < size; i++)
        _hsv2rgb(h[i], s[i], v[i], r + i, g + i, b + i);
}

/**
 * @brief Convert an array from sRGB to Hue-Saturation-Intensity (HSI).
 *
 * This routine uses _rgb2hsi() on a float array with sRGB values
 * in [0-UCHAR_MAX] to produce the equivalent float array
 * with HSI values in [0-6[x[0-1]x[0-1].
 *
 * @param rgb input array
 * @param hsi output array
 * @param size array size
 */
void rgb2hsi(const float *rgb, float *hsi, size_t size)
{
    const float *r, *g, *b;
    float *h, *s, *i;
    size_t j;

    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;
    h = hsi;
    s = hsi + size;
    i = hsi + 2 * size;

    for (j = 0; j < size; j++)
        _rgb2hsi(r[j], g[j], b[j], h + j, s + j, i + j);
}

/**
 * @brief Convert an array from Hue-Saturation-Intensity (HSI) to sRGB.
 *
 * This routine uses _hsi2rgb() on a float array with HSI values in
 * [0-6[x[0-1]x[0-1] to produce the equivalent float array with sRGB
 * values in [0-UCHAR_MAX].
 *
 * @param hsi input array
 * @param rgb output array
 * @param size array size
 */
void hsi2rgb(const float *hsi, float *rgb, size_t size)
{
    const float *h, *s, *i;
    float *r, *g, *b;
    size_t j;

    h = hsi;
    s = hsi + size;
    i = hsi + 2 * size;
    r = rgb;
    g = rgb + size;
    b = rgb + 2 * size;

    for (j = 0; j < size; j++)
        _hsi2rgb(h[j], s[j], i[j], r + j, g + j, b + j);
}

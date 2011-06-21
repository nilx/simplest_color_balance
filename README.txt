% Color balance with saturation

# ABOUT

* Author    : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
              Jose-Luis Lisani <joseluis.lisani@uib.es>
              Catalina Sbert <catalina.sbert@uib.es>
* Copyright : (C) 2009-2011 IPOL Image Processing On Line http://www.ipol.im/
* Licence   : GPL v3+ and BSD, see GPLv3.txt and BSD_simplified.txt

# OVERVIEW

This source code provides an implementation of the "simplest color
balance" image normalization, as described in IPOL
    http://www.ipol.im/pub/algo/lmps_simplest_color_balance/

The implementation is realized with two programs for two variants of
the algorithm. These programs read a PNG image, then scale the pixel
values by an affine function such that a user-defined number of pixels
are scaled over the color space limits [0..255]. These pixels are
replaced by the 0 or 255 value, and these values are written as a PNG
image.

The 'balance_rgb' program operates independently on each of the red,
green and blue channels. The 'balance_hsl' program operates on the L
dimension (lightness) of the image. 

Only 8bit RGB PNG images are handled. Other PNG files are implicitly
converted to 8bit color RGB.

# REQUIREMENTS

The code is written in ANSI C, and should compile on any system with
an ANSI C compiler.

The libpng header and the libpng and zlib libraries are required
on the system for compilation and execution. See
http://www.libpng.org/pub/png/libpng.html

# COMPILATION

Simply use the provided makefile, with the command `make`.
Alternatively, you can manually compile
    cc io_png.c balance_lib.c balance_rgb.c \
        -lpng -o balance_rgb
    cc io_png.c balance_lib.c colorspace_lib.c balance_hsl.c \
        -lpng -o balance_hsl

# USAGE

'balance_rgb' and 'balancd_hsl' takes 4 parameters:
    `balance_xxx Sb Sw in.png out.png`

* `Sb`      : percentage of pixels saturated to black in the output image
* `Sw`      : percentage of pixels saturated to white in the output image
              Sb and Sw must be in [0..100[ and Sb+Sw < 100
* `in.png`  : input image
* `out.png` : output image
              both images are PNG; you can use "-" for standard input/output


# CREDITS

This code uses the color conversion code from Pascal Getreuer's
colorspace library: http://www.math.ucla.edu/~getreuer/colorspace.html

# ABOUT THIS FILE

Copyright 2009-2011 IPOL Image Processing On Line http://www.ipol.im/
Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.

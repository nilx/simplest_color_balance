% Color balance with saturation

# ABOUT

* Author    : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
              Jose-Luis Lisani <joseluis.lisani@uib.es>
              Catalina Sbert <catalina.sbert@uib.es>
* Copyright : (C) 2009-2011 IPOL Image Processing On Line http://www.ipol.im/
* Licence   : GPL v3+, see GPLv3.txt

# OVERVIEW

This source code provides an implementation of the "simplest color
balance" image normalization, as described in IPOL
    http://www.ipol.im/pub/algo/lmps_simplest_color_balance/

This algorithm is implemented in the program 'balance'. This program
reads a PNG image, then scale the pixel values by an affine function
such that a user-defined number of pixels are scaled over the color
space limits [0..255]. These pixels are replaced by the 0 or 255
value, and these values are written as a PNG image.

Two different modes are available for color images:
* RGB: the R, G and B channels are balanced independently;
* IRGB: the balance is performed on the I intensity axis, then
    the RGB channels are scaled proportionally, with a projection on
    the RGB cube.

Only 8bit RGB PNG images files are handled. Other PNG files are
implicitly converted to 8bit color RGB.

# REQUIREMENTS

The code is written in ANSI C, and should compile on any system with
an ANSI C compiler. It has been tested on Linux 64bits with various
compilers.

The libpng header and the libpng and zlib libraries are required
on the system for compilation and execution. See
http://www.libpng.org/pub/png/libpng.html

# COMPILATION

Simply use the provided makefile, with the command `make`. Some of the
default compiler flags in the makefile are specific to the gcc
compiler family and can be avoided by `make CFLAGS=`.
Alternatively, you can manually compile
    cc io_png.c balance_lib.c colorbalance_lib.c \
        balance.c -lpng -o balance

# USAGE

'balance' takes 5 parameters:
    `balance mode Smin Smax in.png out.png`

* `mode`    : the algorithm variant, 'rgb' or 'irgb'
* `Smin`    : percentage of pixels saturated to the min value
* `Smax`    : percentage of pixels saturated to the max value
              Smin and Smax must be in [0..100[ and Smin+Smax < 100
* `in.png`  : input image
* `out.png` : output image
              both images are PNG; you can use "-" for standard input/output

# FILES

* balance.c            : command-line handler
* balance_lib.c/h      : base algorithm in one dimension
* colorbalance_lib.c/h : algorithm variants for color images
* io_png.c/h           : simplified interface to libpng
* makefile             : build configuration
* test                 : automates test scripts
* data                 : example and test images
* GPLv3.txt            : source code license
* doc                  : doxygen files
* README.txt           : this file

# ABOUT THIS FILE

Copyright 2009-2011 IPOL Image Processing On Line http://www.ipol.im/
Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.

% Color balance with saturation

# ABOUT

* Author    : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
              Jose-Luis Lisani <joseluis.lisani@uib.es>
              CAtalina Sbert <catalina.sbert@uib.es>
* Copyright : (C) 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
* Licence   : GPL v3+, see GPLv3.txt

# OVERVIEW

This source code provides an implementation of the "simplest color
balance" image normalization, as described in IPOL
    http://www.ipol.im/pub/algo/lmps_simplest_color_balance/

This implementation uses an histogram method.

This program reads PNG image, then scale the pixel values by an
affine function such that a user-defined number of pixels are scaled
over the [0..255] limits. These pixels are replaced by the 0 or 255
value, and these values are written as a PNG image.

Only 8bit RGB PNG images are handled. Other PNG files are implicitly
converted to 8bit color RGB.

# REQUIREMENTS

The code is written in ANSI C, and should compile on any system with
an ANSI C compiler.

The libpng and header and the libpng and zlib libraries are required
on the system for compilation and execution. See
http://www.libpng.org/pub/png/libpng.html

# COMPILATION

Simply use the provided makefile, with the command `make`.
Alternatively, you can manually compile
    cc io_png.c balance_lib.c balance.c -lpng -o normalize_histo

# USAGE

'balance' takes 5 parameters: `normalize_histo S1 S2 in out1 out2`

* `S1`       : percentage of pixels saturated to 0
               in the output image [0...100[
* `S2`       : percentage of pixels saturated to 255
               in the output image [0...100[
* `in`       : input image
* `out1`     : output image, each color channel processed independently
* `out2`     : output image, R/G/B ratios of the original image preserved 


# ABOUT THIS FILE

Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.

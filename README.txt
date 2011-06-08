% Color balance with saturation

# ABOUT

* Author    : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
* Copyright : (C) 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
* Licence   : GPL v3+, see GPLv3.txt

# OVERVIEW

This source code provides two implementations of the "simplest color
balance" image normalization, as described in IPOL
    http://www.ipol.im/pub/algo/lmps_simplest_color_balance/

* normalize_sort uses a simple sorting method
* normalize_histo uses an histogram method, more efficient and
  requiring less memory

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
    cc io_png.c normalize_histo_lib.c normalize_histo.c \
        -lpng -o normalize_histo

# USAGE

'normalize_sort' takes 3 parameters: `normalize_sort S in out`

* `S`       : saturated pixels percentage [0...100[
* `in`      : input image
* `out`     : output image


'normalize_histo' takes 5 parameters: `normalize_histo S1 S2 in out1 out2`

* `S1`       : percentage of pixels saturated to 0 in the output image [0...100[
* `S2`       : percentage of pixels saturated to 255 in the output image [0...100[
* `in`      : input image
* `out1`     : output image, each color channel processed independently
* `out2`     : output image, R/G/B ratios of the original image preserved 


# ABOUT THIS FILE

Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.


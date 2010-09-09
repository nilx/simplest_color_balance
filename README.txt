% Color balance with saturation

# ABOUT

* Author    : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
* Copyright : (C) 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
* Licence   : GPL v3+, see GPLv3.txt

# OVERVIEW

This source code provides two implementation of the "simplest color
balance" image normalization, as described in IPOL
    http://www.ipol.im/pub/algo/lmps_simplest_color_balance/

* normalize_sort uses a simple sorting method
* normalize_histo uses an histogram method, more efficient and
  requiring less memory

These programs read TIFF image, then scale the pixel values by an
affine function such that a user-defined number of pixels are scaled
over the [0..255] limits. These pixels are replaced by the 0 or 255
value, and these values are written as a TIFF image.

Only 8bit RGB TIFF images are handled. Other TIFF files are implicitly
converted to 8bit color RGB.

# REQUIREMENTS

The code is written in ANSI C, and should compile on any system with
an ANSI C compiler.

The libtiff header and libraries are required on the system for
compilation and execution. See http://www.remotesensing.org/libtiff/ 

# COMPILATION

Simply use the provided makefile, with the command `make`.
Alternatively, you can manually compile
    cc io_tiff.c normalize_histo_lib.c normalize_histo.c \
        -ltiff -o normalize_histo
    cc io_tiff.c normalize_sort.c -ltiff -o normalize_sort

# USAGE

The programs takes 3 parameters: `program S in out`

* `program` : normalize_histo or normalize_sort
* `S`       : saturated pixels percentage [0...100[
* `in`      : input image
* `out`     : output image

# ABOUT THIS FILE

Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.

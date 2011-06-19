#!/bin/sh -e
#
# Check there is no memory leak with valgrind/memcheck.

_test_memcheck() {
    test "0" = "$( valgrind --tool=memcheck $* 2>&1 | grep -c 'LEAK' )"
}

################################################

_log_init

echo "* check memory leaks"
_log make distclean
_log make
_log _test_memcheck ./balance 0 0 data/colors.png \
    /tmp/out_rgb.png /tmp/out_i.png
_log _test_memcheck ./balance 10 20 data/colors.png \
    /tmp/out_rgb.png /tmp/out_i.png
_log _test_memcheck ./balance 50 50 data/colors.png \
    /tmp/out_rgb.png /tmp/out_i.png


_log make distclean

_log_clean

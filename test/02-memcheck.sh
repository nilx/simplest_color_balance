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
TEMPFILE=$(tempfile)
_log _test_memcheck ./balance_rgb 0 0 data/colors.png $TEMPFILE
_log _test_memcheck ./balance_rgb 10 20 data/colors.png $TEMPFILE
_log _test_memcheck ./balance_rgb 50 50 data/colors.png $TEMPFILE
_log _test_memcheck ./balance_hsl 0 0 data/colors.png $TEMPFILE
_log _test_memcheck ./balance_hsl 10 20 data/colors.png $TEMPFILE
_log _test_memcheck ./balance_hsl 50 50 data/colors.png $TEMPFILE
rm -f $TEMPFILE


_log make distclean

_log_clean

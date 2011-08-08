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
for MODE in rgb hsl hsv hsi irgb_bounded; do
    _log _test_memcheck ./balance $MODE 0 0 data/colors_small.png $TEMPFILE
    _log _test_memcheck ./balance $MODE 23 42 data/colors_small.png $TEMPFILE
    _log _test_memcheck ./balance $MODE 50 50 data/colors_small.png $TEMPFILE
done
rm -f $TEMPFILE


_log make distclean

_log_clean

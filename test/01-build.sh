#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE=$(tempfile)
    ./balance_rgb 10 20 data/colors.png $TEMPFILE
    test "64b5d6e2fa9440b04d768b522a78d4bd  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance_rgbf 10 20 data/colors.png $TEMPFILE
    test "64b5d6e2fa9440b04d768b522a78d4bd  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    rm -f $TEMPFILE
}

################################################

_log_init

echo "* default build, clean, rebuild"
_log make distclean
_log make
_log _test_run
_log make
_log make clean
_log make

echo "* compiler support"
for CC in cc c++ gcc g++ tcc clang pathcc; do
    if which $CC; then
	echo "* $CC compiler"
	_log make distclean
	_log make CC=$CC CFLAGS=
	_log _test_run
    fi
done
for CC in icc; do
    if which $CC; then
	echo "* $CC compiler"
	_log make distclean
	_log make CC=$CC CFLAGS=-prec-div
	_log _test_run
    fi
done
for CC in gcc g++ clang; do
    if which $CC; then
	echo "* $CC compiler with flags"
	_log make distclean
	_log make CC=$CC
	_log _test_run
    fi
done

_log make distclean

_log_clean

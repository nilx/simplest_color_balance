#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE_RGB=$(tempfile)
    TEMPFILE_I=$(tempfile)
    ./balance 10 20 data/colors.png $TEMPFILE_RGB $TEMPFILE_I
    test "8ec9ab3d216b50bb018b5a4a80e800fb  $TEMPFILE_RGB" \
	= "$(md5sum $TEMPFILE_RGB)"
    test "c460eed5d7001d17218f6b3496b43923  $TEMPFILE_I" \
	= "$(md5sum $TEMPFILE_I)"
    rm -f $TEMPFILE_RGB $TEMPFILE_I
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
for CC in cc c++ gcc g++ tcc clang icc pathcc; do
    if which $CC; then
	echo "* $CC compiler"
	_log make distclean
	_log make CC=$CC CFLAGS=
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

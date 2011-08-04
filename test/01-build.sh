#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE=$(tempfile)
    ./balance_rgb 10 20 data/colors.png $TEMPFILE
    test "64b5d6e2fa9440b04d768b522a78d4bd  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance_hsl 10 20 data/colors.png $TEMPFILE
    test "d1bad04a9942848383da68390f25ea56  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance_hsi_bounded 10 20 data/colors.png $TEMPFILE
    test "cc5b7d862f573462f591003c445e84e4  $TEMPFILE" \
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
for CC in cc c++ gcc g++ tcc nwcc clang icc pathcc suncc; do
    which $CC || continue
    echo "* $CC compiler"
    _log make distclean
    case $CC in
	"gcc"|"g++")
	    _log make CC=$CC ;;
	"icc")
	    # default icc behaviour is wrong divisions!
	    _log make CC=$CC CFLAGS=-prec-div ;;
	*)
	    _log make CC=$CC CFLAGS= ;;
    esac
    _log _test_run
done

_log make distclean

_log_clean

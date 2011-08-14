#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE=$(tempfile)
    ./balance rgb 10 20 data/colors.png $TEMPFILE
    test "d704c1bcad2fe1d91ea1ba1f05c39efd  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance hsl 10 20 data/colors.png $TEMPFILE
    test "2d3389e3e3b6638c3c43912a58cc2ac4  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance hsv 10 20 data/colors.png $TEMPFILE
    test "3405cb7190fa14ba2d13d547e978c7ee  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance hsi 10 20 data/colors.png $TEMPFILE
    test "830c33409920e0945317f3781f9833f0  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance irgb_bounded 10 20 data/colors.png $TEMPFILE
    test "1e5627064a817a78ed45397a04ea2904  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance irgb_adjusted 10 20 data/colors.png $TEMPFILE
    test "47f13d6e15e50d29e812d623d2ae1316  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    rm -f $TEMPFILE
}

################################################

_log_init

echo "* default build, clean, rebuild"
_log make -B
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
	    _log make CC=$CC CFLAGS="-fp-model precise";;
	*)
	    _log make CC=$CC CFLAGS= ;;
    esac
    _log _test_run
done

_log make distclean

_log_clean

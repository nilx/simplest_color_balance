#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE=$(tempfile)
    ./balance rgb 10 20 data/colors.png $TEMPFILE
    test "dac4d6a6add6aabac2e238f8b1f4713d  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance hsl 10 20 data/colors.png $TEMPFILE
    test "d1bad04a9942848383da68390f25ea56  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance hsv 10 20 data/colors.png $TEMPFILE
#    test "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  $TEMPFILE" \
#	= "$(md5sum $TEMPFILE)"
    ./balance hsi 10 20 data/colors.png $TEMPFILE
#    test "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  $TEMPFILE" \
#	= "$(md5sum $TEMPFILE)"
    ./balance hsi_bounded 10 20 data/colors.png $TEMPFILE
    test "2088a1faf7af5d046545148b592d52bc  $TEMPFILE" \
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

#!/bin/sh
#
# Test the code compilation and execution.

# simple code execution
_test_run() {
    TEMPFILE=$(tempfile)
    ./balance rgb 10 20 data/colors.png $TEMPFILE
    test "3624544f3ba96489eb45d2ae7042e640  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance rgb 10 20 - - < data/colors.png > $TEMPFILE
    test "3624544f3ba96489eb45d2ae7042e640  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance irgb 10 20 data/colors.png $TEMPFILE
    test "4b271d168e536d5a916ba5a03889f763  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    ./balance irgb 10 20 - - < data/colors.png > $TEMPFILE
    test "4b271d168e536d5a916ba5a03889f763  $TEMPFILE" \
	= "$(md5sum $TEMPFILE)"
    rm -f $TEMPFILE
}

################################################

_log_init

echo "* default build, clean, rebuild"
_log make -B debug
_log _test_run
_log make -B
_log _test_run
_log make
_log make clean
_log make

echo "* compiler support"
for CC in cc c++ c89 c99 gcc g++ tcc nwcc clang icc pathcc suncc \
    i586-mingw32msvc-cc; do
    which $CC || continue
    echo "* $CC compiler"
    _log make distclean
    case $CC in
	"i586-mingw32msvc-cc")
	    test -d ./win32/lib || break
	    # default mingw behaviour has extra precision
	    _log make CC=$CC CFLAGS="-O2 -ffloat-store"\
		CPPFLAGS="-DNDEBUG -I. -I./win32/include" \
		LDFLAGS="-L./win32/lib"
	    ln -f -s ./win32/bin/libpng3.dll ./win32/bin/zlib1.dll ./
	    ;;
	"icc")
	    # default icc behaviour is wrong divisions!
	    _log make CC=$CC CFLAGS="-fp-model precise"
	    ;;
	*)
	    _log make CC=$CC
	    ;;
    esac
    _log _test_run
    rm -f *.dll
done

_log make distclean

_log_clean

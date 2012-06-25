# script to build the zenlang amalgamation files
# bash ../../../tools/buildlib.sh dbg

#set -x

mode=rel
dotest=yes

while (( "$#" )); do
    if [ "$1" == "dbg" ]; then
        mode=dbg
    fi
    if [ "$1" == "notest" ]; then
        dotest=no
    fi
    shift
done

platform=`uname`

echo Detecting SDK location for $platform ...
if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
    SDKDIR="xx"
    if [ -z "${VS100COMNTOOLS}" ]; then
        SDKDIR="${VS100COMNTOOLS}"
    elif [ -z "${VS90COMNTOOLS}" ]; then
        SDKDIR="${VS100COMNTOOLS}"
    fi

    SDKDIR="${VS100COMNTOOLS}"

    if [ "${SDKDIR}" == "xx" ]; then
        echo Windows SDK not found.
        exit
    fi

    SDKDIR=$(echo "${SDKDIR}" | sed "s/Tools//g" | sed "s/Common7//g")
    command -v cygpath > /dev/null && SDKDIR=$(cygpath -u "${SDKDIR}")
    CC="${SDKDIR}/VC/bin/cl.exe"
    if [ ! -f "${CC}" ]; then
        echo MSVC compiler not found at: ${CC}.
        exit
    fi
elif [[ $platform == 'Darwin' ]]; then
    SDKDIR=`find /Applications/Xcode.app/ -name usr | grep "MacOSX10.7.sdk/usr$"`
else
    echo "unknown platform"
    exit
fi
echo Found SDK at: ${SDKDIR}

#############################################################
initFile() {
    if [ -z "$1" ]; then
        dst_file=$1
    fi
    dst_file="$1"
    echo Initializing $dst_file
    if [ -f ${dst_file} ]; then
        rm ${dst_file}
    fi
    touch ${dst_file}
}

#############################################################
appendFile() {
    if [ -z "$1" ]; then
        dst_file=$1
    fi
    if [ -z "$2" ]; then
        src_list=$2
    fi
    dst_file="$1"
    src_file="$2"
    echo Appending $dst_file: $src_file
    if [ ! -f ${src_file} ]; then
        echo File does not exist: $src_file
        exit 1
    fi

    if [ "$mode" == "dbg" ]; then
        # get absolute file name of src file (use this when building debug version)
        AFN=""
        if [[ $platform == 'Darwin' ]]; then
            AFN=$src_file # readlink does not work under Darwin
        else
            AFN=$(readlink -f "$src_file")
        fi

        # if running under cygwin, convert cygwin-style path ("/cygdrive/z/xx") to dos-style path ("c:\xx")
        command -v cygpath > /dev/null && AFN=$(cygpath -m "$AFN")

        # generate #line statement
        echo "#line 1 \"$AFN\"" >> ${dst_file}

        # append src_file to dst_file, commenting out all #include statements
        # if any include must be retained, write it as '# include'
        # (with a space between the '#' and the 'include')
        sed 's|#include|//#include|' $src_file | sed 's|#pragma once|//#pragma once|' >> ${dst_file}
    else
        # write src file name to dst file.
        echo "// \"$src_file\"" >> ${dst_file}

        # same as above sed command, additionally also comment out #line statements, if any
        sed 's|#include|//#include|' $src_file | sed 's|#pragma once|//#pragma once|' | sed 's|#line|//#line|' >> ${dst_file}
    fi

}

#############################################################
appendString() {
    if [ -z "$1" ]; then
        dst_file=$1
    fi
    if [ -z "$2" ]; then
        src_text=$2
    fi
    dst_file="$1"
    src_text="$2"
    echo Appending $dst_file: \"$src_text\"
    echo $src_text >> $dst_file
}

#############################################################
if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
    ZCC_FILE=zenlang.exe
    SRCDIR=../../../
    LIBDIR=${SRCDIR}/lib
    BLDDIR=.
    INTDIR=${BLDDIR}
    GENDIR=${BLDDIR}/../
    OUTDIR=../../../../zenlang_bld
elif [[ $platform == 'Linux' ]]; then
    echo Linux not implemented
    exit 1
    ZCC_FILE=zenlang
    SRCDIR=../../../../../zenlang/
    LIBDIR=${SRCDIR}/lib
    BLDDIR=.
    INTDIR=${BLDDIR}
    GENDIR=${BLDDIR}/../
    OUTDIR=../../../../../zenlang_bld
elif [[ $platform == 'Darwin' ]]; then
    ZCC_FILE=zenlang
    SRCDIR=../../../../../../../../
    LIBDIR=${SRCDIR}/lib
    BLDDIR=.
    INTDIR=${BLDDIR}
    GENDIR=../../../../../
    OUTDIR=${SRCDIR}/../zenlang_bld
else
    echo "unknown platform"
    exit 1
fi

#########################################################
# check that zcc exists
if [ ! -f ${BLDDIR}/${ZCC_FILE} ]; then
    echo File does not exist: ${BLDDIR}/${ZCC_FILE}
    exit 1
fi

#############################################################
# make all output directories
mkdir -p ${INTDIR}
mkdir -p ${INTDIR}/core
mkdir -p ${INTDIR}/gui

#rm -rf ${OUTDIR}

mkdir -p ${OUTDIR}
mkdir -p ${OUTDIR}/core
mkdir -p ${OUTDIR}/gui
mkdir -p ${OUTDIR}/utils
mkdir -p ${OUTDIR}/utils/fcgi
mkdir -p ${OUTDIR}/utils/sqlite3

#########################################################
# copy exe to OUTDIR
if [[ $platform == 'Darwin' ]]; then
    cp -v ${BLDDIR}/${ZCC_FILE} ${OUTDIR}/${ZCC_FILE}.osx
    ZCC=${OUTDIR}/${ZCC_FILE}.osx
else
    cp -v ${BLDDIR}/${ZCC_FILE} ${OUTDIR}/
    ZCC=${OUTDIR}/${ZCC_FILE}
fi

#########################################################
# copy utils files to OUTDIR
if [[ $platform == 'Darwin' ]]; then
    cp -v -r ${LIBDIR}/utils/ ${OUTDIR}/utils
else
    cp -v -r ${LIBDIR}/utils/ ${OUTDIR}/
fi

#########################################################
# copy tools files to OUTDIR
cp -v "${SRCDIR}/tools/lemon.exe" ${OUTDIR}/
cp -v "${SRCDIR}/tools/lemon.osx" ${OUTDIR}/
cp -v "${SRCDIR}/tools/lempar.c"  ${OUTDIR}/
cp -v "${SRCDIR}/tools/lemon"     ${OUTDIR}/lemon.linux
cp -v "${SRCDIR}/tools/re2c.exe"  ${OUTDIR}/
cp -v "${SRCDIR}/tools/re2c.osx"  ${OUTDIR}/
cp -v "${SRCDIR}/tools/re2c"      ${OUTDIR}/re2c.linux

#############################################################
#generate all core files
CZPPLST="${CZPPLST} $LIBDIR/core/Application.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/String.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/DateTime.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/File.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Dir.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Url.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Request.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Response.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Socket.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Network.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/FastCGI.zpp"
${ZCC} --api ${INTDIR}/core/ -c ${CZPPLST}
if [[ $? != 0 ]]; then
    echo Error generating core files.
    exit
fi

# copy core files to OUTDIR
cp -v -r ${INTDIR}/core/*.ipp ${OUTDIR}/core

#############################################################
#generate all gui files
GZPPLST="${GZPPLST} $LIBDIR/gui/Widget.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Window.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/MainFrame.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/TextEdit.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Button.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Systray.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Menu.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/MenuItem.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/WindowCreator.zpp"
${ZCC} --api ${INTDIR}/gui -c ${GZPPLST}
if [[ $? != 0 ]]; then
    echo Error generating gui/ files.
    exit
fi

# copy gui files to OUTDIR
cp -v -r ${INTDIR}/gui/*.ipp ${OUTDIR}/gui

#############################################################
# create amalgamated zenlang.hpp
ZHDRFILE=${OUTDIR}/zenlang.hpp
initFile $ZHDRFILE
appendString $ZHDRFILE "#pragma once"
appendFile $ZHDRFILE "${LIBDIR}/base/pch.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/base.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/args.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/ast.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/error.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/unit.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/factory.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/token.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/typename.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/scanner.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/parser.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/lexer.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/compiler.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/generator.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/ZenlangGenerator.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/StlcppGenerator.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/CMakeGenerator.hpp"
appendFile $ZHDRFILE "${LIBDIR}/base/Interpreter.hpp"

appendFile $ZHDRFILE "${INTDIR}/core/Application.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/String.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/DateTime.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/File.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Dir.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Url.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Request.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Response.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Socket.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Network.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/FastCGI.hpp"

appendString $ZHDRFILE "#if defined(GUI)"
appendFile $ZHDRFILE "${INTDIR}/gui/Widget.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Window.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/MainFrame.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/TextEdit.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Button.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Systray.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Menu.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/MenuItem.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/WindowCreator.hpp"
appendString $ZHDRFILE "#endif"

#############################################################
# create amalgamated zenlang.cpp
ZSRCFILE=${OUTDIR}/zenlang.cpp
initFile $ZSRCFILE
appendFile $ZSRCFILE "${LIBDIR}/base/base.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/ast.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/unit.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/factory.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/typename.cpp"
appendFile $ZSRCFILE "${GENDIR}/parserGen.h"
appendFile $ZSRCFILE "${GENDIR}/parserGen.hpp"
appendFile $ZSRCFILE "${LIBDIR}/base/scanner.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/parser.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/lexer.cpp"
appendFile $ZSRCFILE "${GENDIR}/lexerGen.hpp"
appendFile $ZSRCFILE "${LIBDIR}/base/compiler.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/generator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/ZenlangGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/StlcppGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/CMakeGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/Interpreter.cpp"

appendFile $ZSRCFILE "${INTDIR}/Application.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/ApplicationImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/String.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/StringImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/DateTime.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/DateTimeImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Dir.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/DirImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/File.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/FileImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Url.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/UrlImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Request.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/RequestImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Response.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/ResponseImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Socket.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/SocketImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Network.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/NetworkImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/FastCGI.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/FastCGIImpl.cpp"

appendString $ZSRCFILE "#if defined(GUI)"
appendFile $ZSRCFILE "${INTDIR}/Widget.cpp"
appendFile $ZSRCFILE "${INTDIR}/Window.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WindowImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WindowImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MainFrame.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MainFrameImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/TextEdit.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/TextEditImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Button.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/ButtonImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Systray.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/SystrayImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Menu.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MenuItem.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuItemImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/WindowCreator.cpp"
appendString $ZSRCFILE "#endif"

if [ "$dotest" == "yes" ]; then
    #########################################################
    # generate unit test files
    ${ZCC} -c ${SRCDIR}/tests/testBasic.zpp
    if [[ $? != 0 ]]; then
        echo Error generating testBasic files.
        exit
    fi

    ${ZCC} -c ${SRCDIR}/tests/testFcgi.zpp
    if [[ $? != 0 ]]; then
        echo Error generating testFcgi files.
        exit
    fi

    ${ZCC} -c ${SRCDIR}/tests/guiTest.zpp
    if [[ $? != 0 ]]; then
        echo Error generating guiTest files.
        exit
    fi

    #########################################################
    echo Running unit tests...

    # remove existing log file, if any
    if [ -f test.log ]; then
        rm test.log
    fi

    # compile and run tests
    if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
        CFLAGS="/Ox /DWIN32 /DUNIT_TEST /DZ_EXE /EHsc /I${OUTDIR} /W4"

        "${CC}" ${CFLAGS} -c ${OUTDIR}/utils/sqlite3/sqlite3.c ${OUTDIR}/utils/sqlite3/sqlite3_unicode.c
        if [[ $? != 0 ]]; then
            echo Error compiling library files.
            exit
        fi

        "${CC}" ${CFLAGS} /FetestBasic.exe sqlite3.obj sqlite3_unicode.obj ${OUTDIR}/zenlang.cpp testBasic.cpp ws2_32.lib shell32.lib
        if [[ $? != 0 ]]; then
            echo Error compiling testBasic files.
            exit
        fi
        ./testBasic.exe > test.log
        if [[ $? != 0 ]]; then
            echo Error running testBasic.
            exit
        fi

        "${CC}" ${CFLAGS} /FetestFcgi.exe /DSERVER sqlite3.obj sqlite3_unicode.obj ${OUTDIR}/utils/fcgi/fastcgi.cpp ${OUTDIR}/zenlang.cpp testFcgi.cpp ws2_32.lib shell32.lib
        if [[ $? != 0 ]]; then
            echo Error compiling testFcgi files.
            exit
        fi

#        "${CC}" /Fefastcgi.exe /EHsc -DWIN32 -DDEBUG_FASTCGI -I ${OUTDIR} ${OUTDIR}/utils/fcgi/fastcgi.cpp ${OUTDIR}/utils/fcgi/test.cpp ws2_32.lib
#        if [[ $? != 0 ]]; then
#            echo Error compiling fastcgi files.
#            exit
#        fi

        "${CC}" ${CFLAGS} /DGUI /FetestGui.exe ${OUTDIR}/utils/sqlite3/sqlite3.c ${OUTDIR}/utils/sqlite3/sqlite3_unicode.c ${OUTDIR}/zenlang.cpp guiTest.cpp user32.lib gdi32.lib comctl32.lib shell32.lib ws2_32.lib
        if [[ $? != 0 ]]; then
            echo Error compiling testGui files.
            exit
        fi
    elif [[ $platform == 'Darwin' ]]; then
        # first compile the C files
        gcc -c -Os -I${SDKDIR}/include -I${OUTDIR} ${OUTDIR}/utils/sqlite3/sqlite3_unicode.c
        if [[ $? != 0 ]]; then
            exit
        fi

        CFLAGS="-DOSX -DUNIT_TEST -DZ_EXE -Wall -I${OUTDIR} -framework Cocoa -O3"

        # next compile the zenlang.cpp file as an objective-c++ file.
        echo CMD test-1
        gcc -c -x objective-c++ ${CFLAGS} ${OUTDIR}/zenlang.cpp
        if [[ $? != 0 ]]; then
            exit
        fi
        echo The above warnings can safely be ignored.

        # now compile the test file
        echo CMD test-2
        g++ ${CFLAGS} -o test.osx sqlite3_unicode.o zenlang.o testBasic.cpp -lc++ -lsqlite3
        if [[ $? != 0 ]]; then
            exit
        fi

        # run the test file
        echo CMD test-3
        ./test.osx > test.log
        if [[ $? != 0 ]]; then
            exit
        fi

        # now compile the test file
#        echo GUI test-1
#        gcc -c -x objective-c++ ${CFLAGS} -DGUI -framework AppKit ${OUTDIR}/zenlang.cpp
#        if [[ $? != 0 ]]; then
#            exit
#        fi

#        echo GUI test-2
#        g++ ${CFLAGS} -DGUI -O3 -o test.osx -L${SDKDIR}/lib sqlite3_unicode.o zenlang.o guiTest.cpp -lc++ -lsqlite3
#        if [[ $? != 0 ]]; then
#            exit
#        fi
    elif [[ $platform == 'Linux' ]]; then
        echo "TODO: Linux platform"
    else
        echo "unknown platform"
    fi

    # display test result summary
    cat test.log | grep "PASSED"
fi

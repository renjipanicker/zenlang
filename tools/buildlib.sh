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
elif [[ $platform == 'Linux' ]]; then
    SDKDIR='xx'
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
        if [[ $? != 0 ]]; then
            echo Error deleting file: ${dst_file}
            exit
        fi
    fi
    touch ${dst_file}
    if [[ $? != 0 ]]; then
        echo Error creating file: ${dst_file}
        exit
    fi
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

    fn=$(basename "$dst_file")
    ext="${fn##*.}"
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
        if [ "$ext" != "ipp" ]; then
            # generate #line statement
            echo "#line 1 \"$AFN\"" >> ${dst_file}
        else
            echo "//#line 1 \"$AFN\"" >> ${dst_file}
        fi

        # append src_file to dst_file, commenting out all #include statements
        # if any include must be retained, write it as '# include'
        # (with a space between the '#' and the 'include')
        if [ "$ext" != "ipp" ]; then
            sed 's|#include|//#include|' $src_file | sed 's|#pragma once|//#pragma once|' >> ${dst_file}
        else
            sed 's|import|//import|' $src_file | sed 's|namespace|//namespace|' >> ${dst_file}
        fi
    else
        # write src file name to dst file.
        echo "// \"$src_file\"" >> ${dst_file}

        # same as above sed command, additionally also comment out #line statements, if any
        if [ "$ext" != "ipp" ]; then
            sed 's|#include|//#include|' $src_file | sed 's|#pragma once|//#pragma once|' | sed 's|#line|//#line|' >> ${dst_file}
        else
            sed 's|import|//import|' $src_file | sed 's|namespace|//namespace|' >> ${dst_file}
        fi
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
    ZCC_FILE=zenlang
    SRCDIR=../../../zenlang/
    LIBDIR=${SRCDIR}/lib
    BLDDIR=.
    INTDIR=${BLDDIR}
    GENDIR=${BLDDIR}
    OUTDIR=../../../zenlang_bld
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

#rm -rf ${OUTDIR}

mkdir -p ${OUTDIR}
mkdir -p ${OUTDIR}/utils
mkdir -p ${OUTDIR}/utils/fcgi
mkdir -p ${OUTDIR}/utils/sqlite3

#############################################################
# initialize amalgamated zenlang files
ZHDRFILE=${OUTDIR}/zenlang.hpp
initFile $ZHDRFILE
ZSRCFILE=${OUTDIR}/zenlang.cpp
initFile $ZSRCFILE
ZINCFILE=${OUTDIR}/zenlang.ipp
initFile $ZINCFILE

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
#generate all library files
CZPPLST="${CZPPLST} $LIBDIR/z/Application.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/String.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/DateTime.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/File.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Dir.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Url.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Packet.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Request.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Response.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Socket.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Network.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/FastCGI.zpp"

CZPPLST="${CZPPLST} $LIBDIR/z/Widget.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Window.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/MainFrame.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/TextEdit.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Button.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Systray.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/Menu.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/MenuItem.zpp"
CZPPLST="${CZPPLST} $LIBDIR/z/WindowCreator.zpp"

${ZCC} --verbose --api ${INTDIR}/z/ -c ${CZPPLST}
if [[ $? != 0 ]]; then
    echo Error generating library files.
    exit
fi

#############################################################
# create amalgamated zenlang.hpp
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

appendFile $ZHDRFILE "${INTDIR}/z/Application.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/String.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/DateTime.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/File.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Dir.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Url.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Packet.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Request.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Response.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Socket.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Network.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/FastCGI.hpp"

appendString $ZHDRFILE "#if defined(GUI)"
appendFile $ZHDRFILE "${INTDIR}/z/Widget.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Window.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/MainFrame.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/TextEdit.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Button.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Systray.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/Menu.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/MenuItem.hpp"
appendFile $ZHDRFILE "${INTDIR}/z/WindowCreator.hpp"
appendString $ZHDRFILE "#endif"

#############################################################
# create amalgamated zenlang.cpp
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
appendFile $ZSRCFILE "${LIBDIR}/z/ApplicationImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/String.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/StringImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/DateTime.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/DateTimeImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Dir.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/DirImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/File.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/FileImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Url.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/UrlImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Packet.cpp"
appendFile $ZSRCFILE "${INTDIR}/Request.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/RequestImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Response.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/ResponseImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Socket.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/SocketImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Network.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/NetworkImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/FastCGI.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/FastCGIImpl.cpp"

appendString $ZSRCFILE "#if defined(GUI)"
appendFile $ZSRCFILE "${INTDIR}/Widget.cpp"
appendFile $ZSRCFILE "${INTDIR}/Window.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/WindowImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/z/WindowImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MainFrame.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/MainFrameImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/TextEdit.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/TextEditImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Button.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/ButtonImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Systray.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/SystrayImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Menu.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/MenuImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MenuItem.cpp"
appendFile $ZSRCFILE "${LIBDIR}/z/MenuItemImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/WindowCreator.cpp"
appendString $ZSRCFILE "#endif"

appendString $ZINCFILE "namespace z;"
appendFile $ZINCFILE "${INTDIR}/z/Application.ipp"
appendFile $ZINCFILE "${INTDIR}/z/String.ipp"
appendFile $ZINCFILE "${INTDIR}/z/DateTime.ipp"
appendFile $ZINCFILE "${INTDIR}/z/File.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Dir.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Url.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Packet.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Request.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Response.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Socket.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Network.ipp"
appendFile $ZINCFILE "${INTDIR}/z/FastCGI.ipp"

#appendString $ZINCFILE "#if defined(GUI)"
appendFile $ZINCFILE "${INTDIR}/z/Widget.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Window.ipp"
appendFile $ZINCFILE "${INTDIR}/z/MainFrame.ipp"
appendFile $ZINCFILE "${INTDIR}/z/TextEdit.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Button.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Systray.ipp"
appendFile $ZINCFILE "${INTDIR}/z/Menu.ipp"
appendFile $ZINCFILE "${INTDIR}/z/MenuItem.ipp"
appendFile $ZINCFILE "${INTDIR}/z/WindowCreator.ipp"
#appendString $ZINCFILE "#endif"

if [ "$dotest" == "yes" ]; then
    #########################################################
    # generate unit test files
    ${ZCC} -c ${SRCDIR}/tests/testBasic.zpp
    if [[ $? != 0 ]]; then
        echo Error generating testBasic files.
        exit
    fi

    ${ZCC} -c ${SRCDIR}/tests/intTest.zpp
    if [[ $? != 0 ]]; then
        echo Error generating intTest files.
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
    dotest=max
    # remove existing log file, if any
    if [ -f test.log ]; then
        rm test.log
    fi

    # compile and run tests
    if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
        CFLAGS="/Ox /DWIN32 /DUNIT_TEST /DZ_EXE /EHsc /I${OUTDIR} /W4"

        if [ "$dotest" == "max" ]; then
            "${CC}" ${CFLAGS} /FetestBasic.exe sqlite3.obj ${OUTDIR}/zenlang.cpp testBasic.cpp ws2_32.lib shell32.lib
            if [[ $? != 0 ]]; then
                echo Error compiling testBasic files.
                exit
            fi
            ./testBasic.exe > test.log
            if [[ $? != 0 ]]; then
                echo Error running testBasic.
                exit
            fi
        fi

        if [ "$dotest" == "max" ]; then
            "${CC}" ${CFLAGS} /FetestFcgi.exe /DSERVER sqlite3.obj ${OUTDIR}/utils/fcgi/fastcgi.cpp ${OUTDIR}/zenlang.cpp testFcgi.cpp ws2_32.lib shell32.lib
            if [[ $? != 0 ]]; then
                echo Error compiling testFcgi files.
                exit
            fi
        fi

        if [ "$dotest" == "max" ]; then
            "${CC}" /Fefastcgi.exe /EHsc -DWIN32 -DDEBUG_FASTCGI -I ${OUTDIR} ${OUTDIR}/utils/fcgi/fastcgi.cpp ${OUTDIR}/utils/fcgi/test.cpp ws2_32.lib
            if [[ $? != 0 ]]; then
                echo Error compiling fastcgi files.
                exit
            fi
        fi

        if [ "$dotest" == "max" ]; then
            "${CC}" ${CFLAGS} /DGUI /FetestGui.exe ${OUTDIR}/utils/sqlite3/sqlite3.c ${OUTDIR}/zenlang.cpp guiTest.cpp user32.lib gdi32.lib comctl32.lib shell32.lib ws2_32.lib
            if [[ $? != 0 ]]; then
                echo Error compiling testGui files.
                exit
            fi
        fi
    elif [[ $platform == 'Darwin' ]]; then
        # first compile the C files
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
        g++ ${CFLAGS} -o test.osx zenlang.o testBasic.cpp -lc++ -lsqlite3
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
#        g++ ${CFLAGS} -DGUI -O3 -o test.osx -L${SDKDIR}/lib zenlang.o guiTest.cpp -lc++ -lsqlite3
#        if [[ $? != 0 ]]; then
#            exit
#        fi
    elif [[ $platform == 'Linux' ]]; then
        CFLAGS="-DUNIT_TEST -DZ_EXE -Wall -I${OUTDIR} -O3"

        # next compile the zenlang.cpp file as an objective-c++ file.
        echo Compiling zenlang/core
        gcc -c ${CFLAGS} ${OUTDIR}/zenlang.cpp
        if [[ $? != 0 ]]; then
            exit
        fi

        echo Compiling testBasic
        g++ ${CFLAGS} -o test zenlang.o testBasic.cpp -lsqlite3
        if [[ $? != 0 ]]; then
            exit
        fi

        echo Running testBasic
        ./test > test.log
        if [[ $? != 0 ]]; then
            exit
        fi

        echo Compiling intTest
        g++ ${CFLAGS} -o intTest zenlang.o intTest.cpp -lsqlite3
        if [[ $? != 0 ]]; then
            exit
        fi

        #echo Compiling zenlang/gui
        #gcc -c ${CFLAGS} -DGUI `pkg-config --cflags --libs gtk+-3.0` ${OUTDIR}/zenlang.cpp
        if [[ $? != 0 ]]; then
            exit
        fi

        #echo Compiling guiTest
        #g++ ${CFLAGS} -DGUI `pkg-config --cflags --libs gtk+-3.0` -O3 -o guiTest -L${SDKDIR}/lib zenlang.o guiTest.cpp -lsqlite3
        if [[ $? != 0 ]]; then
            exit
        fi
    else
        echo "unknown platform"
    fi

    # display test result summary
    cat test.log | grep "PASSED"
fi

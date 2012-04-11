# script to build the zenlang amalgamation files
dbg=n
if [ "$1" == "dbg" ]; then
    dbg=y
fi

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

    LNO=1
    AFN=""
    if [ "$dbg" == "y" ]; then
        # get absolute file name of src file (use this when building debug version)
        AFN=$(readlink -f "$src_file")
    else
        # get absolute file name of dst file (use this when building release version)
        AFN=$(readlink -f "$dst_file")

        # get line number of dst file
        LNO=$(wc -l ${dst_file} | cut -f1 -d ' ')
        let LNO=$LNO+1
    fi

    command -v cygpath > /dev/null && AFN=$(cygpath -m "$AFN")

    # generate #line statement
    echo "#line $LNO \"$AFN\"" >> ${dst_file}

    # append src_file to dst_file, commenting out all #include statements
    # if any include must be retained, write it as '# include'
    # (with a space between the '#' and the 'include')
    sed 's|#include|//#include|' $src_file >> ${dst_file}
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
SRCDIR=../../../
LIBDIR=${SRCDIR}/lib
BLDDIR=.
INTDIR=${BLDDIR}
OUTDIR=../../../../zenlang_bld
ZCC_FILE=zenlang.exe

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

rm -rf ${OUTDIR}
mkdir -p ${OUTDIR}
mkdir -p ${OUTDIR}/utils
mkdir -p ${OUTDIR}/utils/sqlite3

#########################################################
# copy exe to OUTDIR
cp ${BLDDIR}/${ZCC_FILE} ${OUTDIR}/

#########################################################
# copy utils files to OUTDIR
cp -r ${LIBDIR}/utils/ ${OUTDIR}/

#########################################################
# set ZCC compiler variable
ZCC=${OUTDIR}/${ZCC_FILE}

#############################################################
#generate all core files
CZPPLST="${CZPPLST} $LIBDIR/core/String.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/DateTime.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/File.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Dir.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Url.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Request.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Response.zpp"
CZPPLST="${CZPPLST} $LIBDIR/core/Network.zpp"
${ZCC} --verbose --api ${INTDIR}/core -c ${CZPPLST}

#############################################################
#generate all gui files
GZPPLST="${GZPPLST} $LIBDIR/gui/Widget.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Window.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/MainFrame.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Button.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/TextEdit.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Systray.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/Menu.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/MenuItem.zpp"
GZPPLST="${GZPPLST} $LIBDIR/gui/WindowCreator.zpp"
${ZCC} --verbose --api ${INTDIR}/gui -c ${GZPPLST}

#############################################################
# create amalgamated zenlang.hpp
ZHDRFILE=${OUTDIR}/zenlang.hpp
initFile $ZHDRFILE
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
appendFile $ZHDRFILE "${INTDIR}/core/String.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/DateTime.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/File.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Dir.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Url.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Request.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Response.hpp"
appendFile $ZHDRFILE "${INTDIR}/core/Network.hpp"

appendString $ZHDRFILE "#if defined(GUI)"
appendFile $ZHDRFILE "${INTDIR}/gui/Widget.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Window.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/MainFrame.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/Button.hpp"
appendFile $ZHDRFILE "${INTDIR}/gui/TextEdit.hpp"
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
appendFile $ZSRCFILE "${LIBDIR}/base/parserGen.h"
appendFile $ZSRCFILE "${LIBDIR}/base/parserGen.hpp"
appendFile $ZSRCFILE "${LIBDIR}/base/scanner.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/parser.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/lexer.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/lexerGen.hpp"
appendFile $ZSRCFILE "${LIBDIR}/base/compiler.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/generator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/ZenlangGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/StlcppGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/CMakeGenerator.cpp"
appendFile $ZSRCFILE "${LIBDIR}/base/Interpreter.cpp"
appendFile $ZSRCFILE "${INTDIR}/String.cpp"
appendFile $ZSRCFILE "${LIBDIR}/core/StringImpl.cpp"

appendString $ZSRCFILE "#if defined(GUI)"
appendFile $ZSRCFILE "${INTDIR}/Widget.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WidgetImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WidgetImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Window.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WindowImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/WindowImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MainFrame.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MainFrameImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Button.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/ButtonImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/TextEdit.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/TextEditImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Systray.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/SystrayImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/SystrayImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/Menu.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/MenuItem.cpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuItemImpl.hpp"
appendFile $ZSRCFILE "${LIBDIR}/gui/MenuItemImpl.cpp"
appendFile $ZSRCFILE "${INTDIR}/WindowCreator.cpp"
appendString $ZSRCFILE "#endif"

#########################################################
# generate unit test files
${ZCC} --verbose -c ${SRCDIR}/tests/testBasic.zpp
${ZCC} --verbose -c ${SRCDIR}/tests/guiTest.zpp

#########################################################
# build and run unit tests
platform=`uname`
if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
    cl /EP /DWIN32 /DUNIT_TEST /DZ_EXE /EHsc /I${OUTDIR} /Fetest.exe ${OUTDIR}/utils/sqlite3/sqlite3.c ${OUTDIR}/utils/sqlite3/sqlite3_unicode.c ${OUTDIR}/zenlang.cpp testBasic.cpp > x.cpp
    ./test.exe > test.log
    cl /DWIN32 /DUNIT_TEST /DZ_EXE /DGUI /EHsc /I${OUTDIR} /FetestGui.exe ${OUTDIR}/utils/sqlite3/sqlite3.c ${OUTDIR}/utils/sqlite3/sqlite3_unicode.c ${OUTDIR}/zenlang.cpp guiTest.cpp user32.lib gdi32.lib comctl32.lib shell32.lib ws2_32.lib
elif [[ $platform == 'Linux' ]]; then
    echo "TODO: Linux platform"
elif [[ $platform == 'Darwin' ]]; then
    echo "TODO: OSX platform"
else
    echo "unknown platform"
fi

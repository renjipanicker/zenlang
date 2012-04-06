# script to build the zenlang amalgamation files

#############################################################
appendFile() {
    if [ -z "$1" ]; then
        dst_file=$1
    fi
    if [ -z "$2" ]; then
        src_list=$2
    fi
    dst_file="$1"
    src_list="$2"
    echo dst_file is $dst_file
    echo src_list is $src_list

    if [ -f ${dst_file} ]; then
        rm ${dst_file}
    fi
    touch ${dst_file}
    for FN in $src_list; do
        echo FN is $FN
        # AFN=$(readlink -f "$FN")
        AFN=$(readlink -f "$dst_file")
        LNO=$(wc -l ${dst_file} | cut -f1 -d ' ')
        let LNO=$LNO+1
        echo "#line $LNO \"$AFN\"" >> ${dst_file}
        if [ -f ${FN} ]; then
            cat $FN >> ${dst_file}
        else
            echo File does not exist: $FN
        fi
    done
}

#############################################################
export SRCDIR=../../../lib
export INTDIR=.
export OUTDIR=../../../../zenlang_bld

mkdir -p ${INTDIR}
mkdir -p ${INTDIR}/core
mkdir -p ${INTDIR}/gui
mkdir -p ${OUTDIR}

export CZPPLST="${CZPPLST} $SRCDIR/core/String.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/DateTime.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/File.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/Dir.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/Url.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/Request.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/Response.zpp"
export CZPPLST="${CZPPLST} $SRCDIR/core/Network.zpp"

${INTDIR}/zenlang --verbose --api ${INTDIR}/core -c ${CZPPLST}

export HDRLST="${HDRLST} ${SRCDIR}/base/pch.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/base.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/ast.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/error.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/unit.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/factory.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/token.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/typename.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/parser.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/lexer.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/compiler.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/generator.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/ZenlangGenerator.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/StlcppGenerator.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/CMakeGenerator.hpp"
export HDRLST="${HDRLST} ${SRCDIR}/base/Interpreter.hpp"

export HDRLST="${HDRLST} ${INTDIR}/core/String.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/DateTime.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/File.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/Dir.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/Url.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/Request.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/Response.hpp"
export HDRLST="${HDRLST} ${INTDIR}/core/Network.hpp"

OFN=${OUTDIR}/zenlang.hpp
appendFile $OFN "$HDRLST"

# generate zenlang.cpp file
export SRCLST="${SRCLST} ${SRCDIR}/base/base.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/ast.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/unit.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/factory.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/typename.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/parserGen.h"
export SRCLST="${SRCLST} ${SRCDIR}/base/parserGen.hpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/parser.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/lexer.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/compiler.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/ZenlangGenerator.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/StlcppGenerator.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/CMakeGenerator.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/base/Interpreter.cpp"

export SRCLST="${SRCLST} ${INTDIR}/String.cpp"
export SRCLST="${SRCLST} ${SRCDIR}/core/StringImpl.cpp"

cat ${SRCLST} > ${OUTDIR}/zenlang.cpp

export BINLST="${BINLST} ${INTDIR}/zenlang.exe"
cp ${BINLST} ${OUTDIR}

platform=`uname`
if [[ $platform == 'CYGWIN_NT-5.1' ]]; then
    cl -c -DWIN32 -EHsc ${OUTDIR}/zenlang.cpp
elif [[ $platform == 'Linux' ]]; then
    echo "Linux platform"
elif [[ $platform == 'Darwin' ]]; then
    echo "OSX platform"
else
    echo "unknown platform"
fi

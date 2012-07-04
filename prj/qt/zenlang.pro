QT -= core
QT -= gui

TARGET  = zenlang
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

DEFINES+=Z_EXE
DEFINES+=ZPP_EXE
INCLUDEPATH+=../../lib/
INCLUDEPATH+=$${DESTDIR}}

##############################################################
# Lexer
re2c.name = re2c
re2c.input = RE2C_SOURCES
re2c.commands = ../../tools/re2c -f -u -c -i -o ${QMAKE_FILE_BASE}.hpp ${QMAKE_FILE_IN}
re2c.output = ${QMAKE_FILE_BASE}.hpp
re2c.variable_out = SOURCES_X
re2c.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += re2c

##############################################################
# Parser
lemon.name = lemon
lemon.input = LEMON_SOURCES
lemon.commands = ../../tools/lemon -o.hpp -q ${QMAKE_FILE_IN}
lemon.output = ${QMAKE_FILE_BASE}.hpp
lemon.variable_out = SOURCES_X
lemon.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += lemon

##############################################################
LEMON_SOURCES += ../../lib/base/parserGen.y
RE2C_SOURCES  += ../../lib/base/lexerGen.re

##############################################################
HEADERS+=../../lib/base/args.hpp
HEADERS+=../../lib/base/ast.hpp
HEADERS+=../../lib/base/base.hpp
HEADERS+=../../lib/base/CmakeGenerator.hpp
HEADERS+=../../lib/base/compiler.hpp
HEADERS+=../../lib/base/error.hpp
HEADERS+=../../lib/base/factory.hpp
HEADERS+=../../lib/base/generator.hpp
HEADERS+=../../lib/base/Interpreter.hpp
HEADERS+=../../lib/base/lexer.hpp
HEADERS+=../../lib/base/MsvcGenerator.hpp
HEADERS+=../../lib/base/parser.hpp
HEADERS+=../../lib/base/pch.hpp
HEADERS+=../../lib/base/scanner.hpp
HEADERS+=../../lib/base/StlcppGenerator.hpp
HEADERS+=../../lib/base/token.hpp
HEADERS+=../../lib/base/typename.hpp
HEADERS+=../../lib/base/unit.hpp
HEADERS+=../../lib/base/ZenlangGenerator.hpp

##############################################################
SOURCES += ../../src/main.cpp
SOURCES += ../../lib/base/ast.cpp
SOURCES += ../../lib/base/base.cpp
SOURCES += ../../lib/base/CmakeGenerator.cpp
SOURCES += ../../lib/base/compiler.cpp
SOURCES += ../../lib/base/factory.cpp
SOURCES += ../../lib/base/generator.cpp
SOURCES += ../../lib/base/Interpreter.cpp
SOURCES += ../../lib/base/lexer.cpp
SOURCES += ../../lib/base/MsvcGenerator.cpp
SOURCES += ../../lib/base/parser.cpp
SOURCES += ../../lib/base/scanner.cpp
SOURCES += ../../lib/base/StlcppGenerator.cpp
SOURCES += ../../lib/base/typename.cpp
SOURCES += ../../lib/base/unit.cpp
SOURCES += ../../lib/base/ZenlangGenerator.cpp

#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/compiler.hpp"
#include "base/ZenlangGenerator.hpp"
#include "base/StlcppGenerator.hpp"

inline bool checkFile(const z::string& filename) {
    return std::ifstream( z::s2e(filename).c_str() ).is_open();
}

inline z::string z::Compiler::findImport(const z::string& filename) {
    z::string cfile;

    // first check current dir
    cfile = "./" + filename;
    if(checkFile(cfile))
        return cfile;

    // next check zen lib dir
    cfile = _config.zlibPath() + "/include/" + filename;
    if(checkFile(cfile))
        return cfile;

    // then all other include paths
    for(z::Ast::Config::PathList::const_iterator it = _config.includePathList().begin(); it != _config.includePathList().end(); ++it) {
        const z::string& dir = *it;
        cfile = dir + "/" + filename;
        if(checkFile(cfile))
            return cfile;
    }
    throw z::Exception("Compiler", zfmt(z::Ast::Token(filename, 0, 0, ""), z::string("Cannot open include file: %{s}").arg("s", filename)));
}

bool z::Compiler::compileFile(z::Ast::Module& module, const z::string& filename, const z::string& msg) {
    if(_project.verbosity() >= z::Ast::Project::Verbosity::Normal) {
        z::string indent = "   ";
        for(z::Ast::Module::Level_t i = 0; i < module.level(); ++i) {
            indent += "  ";
        }
        indent += msg;
    }

    std::ifstream is;
    is.open(z::s2e(filename).c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception("Compiler", zfmt(z::Ast::Token(filename, 0, 0, ""), z::string("Error opening file %{s}").arg("s", filename)));
    }

    Parser parser;
    Lexer lexer(parser);
    z::Ast::Factory factory(module);
    ParserContext pctx(factory, z::ref(this));
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        std::streamsize got = is.gcount();
        lexer.push(pctx, buf, got, is.eof());
    }
    return true;
}

inline bool z::Compiler::parseFile(z::Ast::Module& module, const z::string& msg) {
    return compileFile(module, module.filename(), msg);
}

void z::Compiler::import(z::Ast::Module& module) {
    z::string ifilename = findImport(module.filename());

    // check if file is already imported
    if(module.unit().headerFileList().find(ifilename) != module.unit().headerFileList().end()) {
        return;
    }

    // if not, add it to list of files imported into this unit
    module.unit().addheaderFile(ifilename);
    compileFile(module, ifilename, "Importing");
}

void z::Compiler::initContext(z::Ast::Unit& unit) {
    z::Ast::Module module(unit, "core/core.ipp", 1);
    z::Ast::Factory factory(module);
    factory.initUnit();
}

void z::Compiler::compile() {
    for(z::Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;

        z::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != z::string::npos) {
            z::Ast::Unit unit;
            initContext(unit);

            z::Ast::Module module(unit, filename, 0);
            if(!compileFile(module, filename, "Compiling"))
                throw z::Exception("Compiler", zfmt(Ast::Token(filename, 0, 0, ""), z::string("Cannot open source file: %{s}").arg("s", filename)));

            ZenlangGenerator zgenerator(_project, _config, module);
            zgenerator.run();

            if(_config.olanguage() == "stlcpp") {
                StlcppGenerator generator(_project, _config, module);
                generator.run();
            } else {
                throw z::Exception("Compiler", zfmt(Ast::Token(filename, 0, 0, ""), "Unknown code generator %{s}").arg("s", _config.olanguage()));
            }
        }
    }
}

void z::Compiler::compileString(Ast::Module& module, Lexer& lexer, const z::string& data, const bool& isEof) {
    z::Ast::Factory factory(module);
    ParserContext pctx(factory, z::ref(this));

    const z::estring edata = z::s2e(data);
    lexer.push(pctx, edata.c_str(), edata.size(), isEof);
}

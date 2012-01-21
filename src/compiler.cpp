#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "ZenlangGenerator.hpp"
#include "StlcppGenerator.hpp"

inline bool checkFile(const std::string& filename) {
    return std::ifstream( filename.c_str() ).is_open();
}

inline std::string Compiler::findImport(const std::string& filename) {
    std::string cfile;

    // first check current dir
    cfile = "./" + filename;
    if(checkFile(cfile))
        return cfile;

    // next check zen lib dir
    cfile = _config.zlibPath() + "/" + filename;
    if(checkFile(cfile))
        return cfile;

    // then all other include paths
    for(Ast::Config::PathList::const_iterator it = _config.includePathList().begin(); it != _config.includePathList().end(); ++it) {
        const std::string& dir = *it;
        cfile = dir + "/" + filename;
        if(checkFile(cfile))
            return cfile;
    }
    throw z::Exception("Compiler", z::fmt("Cannot open include file %{s}").add("s", filename));
}

bool Compiler::compileFile(Ast::Module& module, const std::string& filename, const std::string& msg) {
    if(_project.verbosity() >= Ast::Project::Verbosity::Normal) {
        std::string indent = "   ";
        for(size_t i = 0; i < module.level(); ++i) {
            indent += "  ";
        }
        indent += msg;
        printf("-- %s %s\n", indent.c_str(), filename.c_str());
    }

    std::ifstream is;
    is.open(filename.c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception("Compiler", z::fmt("Error opening file %{s}").add("s", filename));
    }

    Parser parser;
    Lexer lexer(parser);
    Ast::NodeFactory factory(module, z::ref(this));
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        size_t got = is.gcount();
        lexer.push(factory, buf, got, is.eof());
    }
    return true;
}

inline bool Compiler::parseFile(Ast::Module& module, const std::string& msg) {
    return compileFile(module, module.filename(), msg);
}

void Compiler::import(Ast::Module& module) {
    std::string ifilename = findImport(module.filename());

    // check if file is already imported
    if(module.unit().headerFileList().find(ifilename) != module.unit().headerFileList().end()) {
        return;
    }

    // if not, add it to list of files imported into this unit
    module.unit().addheaderFile(ifilename);
    compileFile(module, ifilename, "Importing");
}

void Compiler::initContext(Ast::Unit& unit) {
    Ast::Module module(unit, "core/core.ipp", 1);
    import(module);
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const std::string& filename = *it;

        std::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != std::string::npos) {
            Ast::Unit unit;
            initContext(unit);

            Ast::Module module(unit, filename, 0);
            if(!compileFile(module, filename, "Compiling"))
                throw z::Exception("Compiler", z::fmt("Cannot open source file %{s}").add("s", filename));

            ZenlangGenerator zgenerator(_project, _config, module);
            zgenerator.run();

            if(_config.olanguage() == "stlcpp") {
                StlcppGenerator generator(_project, _config, module);
                generator.run();
            } else {
                throw z::Exception("Compiler", z::fmt("Unknown code generator %{s}").add("s", _config.olanguage()));
            }
        }
    }
}

void Compiler::compileString(Ast::Module& module, Lexer& lexer, const std::string& data, const bool& isEof) {
    Ast::NodeFactory factory(module, z::ref(this));
    lexer.push(factory, data.c_str(), data.size(), isEof);
}

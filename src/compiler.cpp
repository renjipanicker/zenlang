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
    throw z::Exception("Cannot open include file '%s'\n", filename.c_str());
}

bool Compiler::parseFile(Ast::Context& ctx, Ast::Unit& unit, Lexer& lexer, const std::string& filename, const int& level, const std::string& msg) {
    if(_project.verbosity() >= Ast::Project::Verbosity::Normal) {
        std::string indent = "   ";
        for(int i = 0; i < level; ++i) {
            indent += "  ";
        }
        indent += msg;
        printf("-- %s %s\n", indent.c_str(), filename.c_str());
    }

    std::ifstream is;
    is.open(filename.c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception("Error opening file '%s'\n", filename.c_str());
    }

    Ast::NodeFactory factory(ctx, z::ref(this), unit, level);
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        size_t got = is.gcount();
        lexer.push(factory, buf, got, is.eof());
    }
    return true;
}

inline bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level, const std::string& msg) {
    Parser parser;
    Lexer lexer(parser);
    Ast::Context ctx(unit, filename);
    return parseFile(ctx, unit, lexer, filename, level, msg);
}

void Compiler::import(Ast::Unit& unit, const std::string &filename, const int& level) {
    std::string ifilename = findImport(filename);

    // check if file is already imported
    if(unit.headerFileList().find(filename) != unit.headerFileList().end()) {
        if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
            printf(" - skipped\n");
        }
        return;
    }

    // if not, add it to list of files imported into this unit
    unit.addheaderFile(filename);
    if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
        printf("\n");
    }

    parseFile(unit, ifilename, level+1, "Importing");
}

void Compiler::initContext(Ast::Unit& unit) {
    import(unit, "core/core.ipp", 0);
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const std::string& filename = *it;

        std::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != std::string::npos) {
            Ast::Unit unit(filename);
            initContext(unit);

            if(!parseFile(unit, filename, 0, "Compiling"))
                throw z::Exception("Cannot open source file '%s'\n", filename.c_str());

            ZenlangGenerator zgenerator(_project, _config, unit);
            zgenerator.run();

            if(_config.olanguage() == "stlcpp") {
                StlcppGenerator generator(_project, _config, unit);
                generator.run();
            } else {
                throw z::Exception("Unknown code generator '%s'\n", _config.olanguage().c_str());
            }
        }
    }
}

void Compiler::parseString(Ast::Context& ctx, Lexer& lexer, Ast::Unit& unit, const std::string& data, const int& level, const bool& isEof) {
    Ast::NodeFactory factory(ctx, z::ref(this), unit, level);
    lexer.push(factory, data.c_str(), data.size(), isEof);
}

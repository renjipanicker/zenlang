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

bool Compiler::parseFile(Lexer& lexer, Ast::Module& module, const std::string& filename, const int& level) {
    std::ifstream is;
    is.open(filename.c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception("Error opening file '%s'\n", filename.c_str());
    }

    // if importing files...
    if(level > 0) {
        if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
            std::string msg = "   ";
            for(int i = 0; i < level; ++i) {
                msg += "  ";
            }
            printf("%s Importing %s", msg.c_str(), filename.c_str());
        }

        // check if file is already imported
        if(module.unit().headerFileList().find(filename) != module.unit().headerFileList().end()) {
            if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
                printf(" - skipped\n");
            }
            return true;
        }

        // if not, add it to list of files imported into this unit
        module.unit().addheaderFile(filename);
        if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
            printf("\n");
        }
    }

    Ast::NodeFactory factory(z::ref(this), module, level, filename);
    while(!is.eof()) {
        char buf[1024];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        size_t got = is.gcount();
        lexer.push(factory, buf, got, is.eof());
    }
    return true;
}

bool Compiler::parseFile(Ast::Module& module, const std::string& filename, const int& level) {
    Parser parser;
    Lexer lexer(parser);
    return parseFile(lexer, module, filename, level);
}

void Compiler::import(Ast::Module& module, const std::string &filename, const int& level) {
    std::string ifilename = findImport(filename);
    parseFile(module, ifilename, level+1);
}

void Compiler::initContext(Ast::Unit& unit) {
    Ast::Module module(unit);
    import(module, "core/core.ipp", 0);
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const std::string& filename = *it;
        if(_project.verbosity() >= Ast::Project::Verbosity::Normal) {
            printf("-- Compiling %s\n", filename.c_str());
        }

        std::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != std::string::npos) {
            Ast::Unit unit(filename);
            initContext(unit);

            Ast::Module module(unit);
            if(!parseFile(module, filename, 0))
                throw z::Exception("Cannot open source file '%s'\n", filename.c_str());

            ZenlangGenerator zgenerator(_project, _config, module);
            zgenerator.run();

            if(_config.olanguage() == "stlcpp") {
                StlcppGenerator generator(_project, _config, module);
                generator.run();
            } else {
                throw z::Exception("Unknown code generator '%s'\n", _config.olanguage().c_str());
            }
        }
    }
}

void Compiler::parseString(Lexer& lexer, Ast::Module& module, const std::string& data, const int& level) {
    Ast::NodeFactory factory(z::ref(this), module, level, "string");
    lexer.push(factory, data.c_str(), data.size(), true);
}

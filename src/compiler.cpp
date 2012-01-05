#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "ZenlangGenerator.hpp"
#include "StlcppGenerator.hpp"

bool Compiler::parseFile(Ast::Module& module, const std::string& filename, const int& level) {
    Ast::NodeFactory factory(z::ref(this), module, level, filename);
    Parser parser(factory);
    Lexer lexer(factory, parser);
//    if(!lexer.openFile(filename))
//        return false;
//    lexer.init();

    std::ifstream is;
    is.open(filename.c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        return false;
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
//    return lexer.read();

    while(!is.eof()) {
        char buf[1024];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        size_t got = is.gcount();
        printf("compiler: got = %lu\n", got);
        if(lexer.push(buf, got, is.eof()) == false) {
            return false;
        }
    }
    return true;
}

void Compiler::import(Ast::Module& module, const std::string &filename, const int& level) {
    // first check current dir
    if(parseFile(module, "./" + filename, level+1))
        return;

    // next check zen lib dir
    if(parseFile(module, _config.zlibPath() + "/" + filename, level+1))
        return;

    // then all other include paths
    for(Ast::Config::PathList::const_iterator it = _config.includePathList().begin(); it != _config.includePathList().end(); ++it) {
        const std::string& dir = *it;
        if(parseFile(module, dir + "/" + filename, level+1))
            return;
    }
    throw z::Exception("Cannot open include file '%s'\n", filename.c_str());
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

bool Compiler::parseString(Ast::Module& module, const std::string& data, const int& level) {
    Ast::NodeFactory factory(z::ref(this), module, level, "string");
    Parser parser(factory);
    Lexer lexer(factory, parser);
//    lexer.init();
//    if(!lexer.openString(data))
//        return false;
    return lexer.push(data.c_str(), data.size(), true);
}


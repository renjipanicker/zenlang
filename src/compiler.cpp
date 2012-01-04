#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "ZenlangGenerator.hpp"
#include "StlcppGenerator.hpp"

bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level) {
    Ast::NodeFactory factory(z::ref(this), unit, level, filename);
    Parser parser(factory);
    Lexer lexer(factory, parser);
    if(!lexer.openFile(filename))
        return false;

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
        if(unit.headerFileList().find(filename) != unit.headerFileList().end()) {
            if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
                printf(" - skipped\n");
            }
            return true;
        }

        // if not, add it to list of files imported into this unit
        unit.addheaderFile(filename);
        if(_project.verbosity() >= Ast::Project::Verbosity::Detailed) {
            printf("\n");
        }
    }

    return lexer.read();
}

void Compiler::import(Ast::Unit& unit, const std::string &filename, const int& level) {
    // first check current dir
    if(parseFile(unit, "./" + filename, level+1))
        return;

    // next check zen lib dir
    if(parseFile(unit, _config.zlibPath() + "/" + filename, level+1))
        return;

    // then all other include paths
    for(Ast::Config::PathList::const_iterator it = _config.includePathList().begin(); it != _config.includePathList().end(); ++it) {
        const std::string& dir = *it;
        if(parseFile(unit, dir + "/" + filename, level+1))
            return;
    }
    throw z::Exception("Cannot open include file '%s'\n", filename.c_str());
}

void Compiler::initContext(Ast::Unit& unit) {
    import(unit, "core/core.ipp", 0);
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

            if(!parseFile(unit, filename, 0))
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

bool Compiler::parseString(Ast::Unit& unit, const std::string& data, const int& level) {
    Ast::NodeFactory factory(z::ref(this), unit, level, "string");
    Parser parser(factory);
    Lexer lexer(factory, parser);
    if(!lexer.openString(data))
        return false;
    return lexer.read();
}


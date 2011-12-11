#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "generator.hpp"
#include "outfile.hpp"

bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level) {
    Context context(ref(this), unit, level, filename);
    Parser parser(context);
    Lexer lexer(context, parser);
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

    return lexer.readFile();
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
    throw Exception("Cannot open include file '%s'\n", filename.c_str());
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
            import(unit, "core/core.ipp", 0);
            if(!parseFile(unit, filename, 0))
                throw Exception("Cannot open source file '%s'\n", filename.c_str());
            Generator generator(_project, _config, unit);
            generator.run();
        }
    }
}

#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "generator.hpp"
#include "progen.hpp"
#include "outfile.hpp"

bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level, const std::string& action) {
    Context context(ref(this), unit, level, filename);
    Parser parser(context);
    Lexer lexer(context, parser);
    if(!lexer.openFile(filename))
        return false;

    std::string msg;
    for(int i = 0; i < level; ++i) {
        msg += "  ";
    }
    if(level == 0)
        msg += "Compiling";
    else
        msg += "Importing";

    // if importing files...
    bool skip = true;
    if(level > 0) {
        // check if file is already imported
        if(unit.headerFileList().find(filename) == unit.headerFileList().end()) {
            // if not, add it to list of files imported into this unit
            unit.addheaderFile(filename);
            skip = false;
        }
    } else {
        skip = false;
    }

    printf("%s %s", msg.c_str(), filename.c_str());
    if(skip) {
        printf(" - skipped\n");
        return true;
    }

    printf("\n");
    return lexer.readFile();
}

void Compiler::import(Ast::Unit& unit, const std::string &filename, const int& level) {
    // first check current dir
    if(parseFile(unit, "./" + filename, level+1, "  Importing"))
        return;

    // next check zen lib dir
    if(parseFile(unit, _project.zlibPath() + "/" + filename, level+1, "  Importing"))
        return;

    // then all other include paths
    for(Ast::Config::PathList::const_iterator it = _project.global().includePathList().begin(); it != _project.global().includePathList().end(); ++it) {
        const std::string& dir = *it;
        if(parseFile(unit, dir + "/" + filename, level+1, "  Importing"))
            return;
    }
    throw Exception("Cannot open include file '%s'\n", filename.c_str());
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _project.global().sourceFileList().begin(); it != _project.global().sourceFileList().end(); ++it) {
        const std::string& filename = *it;
        std::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != std::string::npos) {
            Ast::Unit unit(filename);
            import(unit, "core/core.ipp", 0);
            if(!parseFile(unit, filename, 0, "Compiling"))
                throw Exception("Cannot open source file '%s'\n", filename.c_str());
            Generator generator(_project, unit);
            generator.run();
        }
    }

    if(_project.mode() != Ast::Project::Mode::Compile) {
        ProGen progen(_project);
        progen.run();
    }
}

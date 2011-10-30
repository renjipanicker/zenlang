#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "generator.hpp"
#include "progen.hpp"

bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level) {
    Context context(ref(this), unit, level);
    Parser parser(context);
    Lexer lexer(context, parser);
    return lexer.readFile(filename);
}

void Compiler::import(Ast::Unit& unit, const std::string &filename, const int& level) {
    trace("importing %s\n", filename.c_str());

    // first check current dir
    if(parseFile(unit, "./" + filename, level+1))
        return;

    // next check zen lib dir
    if(parseFile(unit, _project.zenPath() + "/" + filename, level+1))
        return;

    // then all other include paths
    for(Ast::Config::PathList::const_iterator it = _project.global().includePathList().begin(); it != _project.global().includePathList().end(); ++it) {
        const std::string& dir = *it;
        if(parseFile(unit, dir + "/" + filename, level+1))
            return;
    }
    throw Exception("Cannot open include file '%s'\n", filename.c_str());
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _project.global().sourceFileList().begin(); it != _project.global().sourceFileList().end(); ++it) {
        const std::string& filename = *it;
        trace("compiling %s\n", filename.c_str());
        Ast::Unit unit(filename);
        import(unit, "core/core.ipp", 0);
        if(!parseFile(unit, filename, 0))
            throw Exception("Cannot open source file '%s'\n", filename.c_str());
        Generator generator(_project, unit);
        generator.run();
    }

    if(_project.mode() != Ast::Project::Mode::Compile) {
        ProGen progen(_project);
        progen.run();
    }
}

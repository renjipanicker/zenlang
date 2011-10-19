#include "base/pch.hpp"
#include "base/common.hpp"
#include "base/exception.hpp"
#include "compiler.hpp"
#include "lexer.hpp"
#include "generator.hpp"

bool Compiler::parseFile(Ast::Unit& unit, const std::string& filename, const int& level) {
    Context context(ref(this), unit, level);
    Parser parser(context);
    Lexer lexer(context, parser);
    return lexer.readFile(filename);
}

void Compiler::import(Ast::Unit& unit, const std::string &filename, const int& level) {
    trace("importing %s\n", filename.c_str());
    for(Project::PathList::const_iterator it = _project.includeList().begin(); it != _project.includeList().end(); ++it) {
        const std::string& dir = *it;
        if(parseFile(unit, dir + "/" + filename, level+1))
            return;
    }
    throw Exception("Cannot open include file '%s'\n", filename.c_str());
}

void Compiler::compile() {
    for(Project::PathList::const_iterator it = _project.sourceList().begin(); it != _project.sourceList().end(); ++it) {
        const std::string& filename = *it;
        trace("compiling %s\n", filename.c_str());
        Ast::Unit unit(Ast::Token(0, 0, filename));
        import(unit, "core/core.ipp", 0);
        if(!parseFile(unit, filename, 0))
            throw Exception("Cannot open source file '%s'\n", filename.c_str());
        Generator generator(_project, unit);
        generator.run();
    }
}

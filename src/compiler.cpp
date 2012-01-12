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

bool Compiler::compileFile(Ast::Context& ctx, Ast::Module& module, Lexer& lexer, const std::string& filename, const int& level, const std::string& msg) {
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

    Ast::NodeFactory factory(ctx, z::ref(this), module, level);
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        size_t got = is.gcount();
        lexer.push(factory, buf, got, is.eof());
    }
    return true;
}

inline bool Compiler::parseFile(Ast::Context& ctx, Ast::Module& module, const std::string& filename, const int& level, const std::string& msg) {
    Parser parser;
    Lexer lexer(parser);
    return compileFile(ctx, module, lexer, filename, level, msg);
}

void Compiler::import(Ast::Context& ctx, Ast::Module& module, const std::string &filename, const int& level) {
    std::string ifilename = findImport(filename);

    // check if file is already imported
    if(ctx.headerFileList().find(filename) != ctx.headerFileList().end()) {
        return;
    }

    // if not, add it to list of files imported into this unit
    ctx.addheaderFile(filename);
    parseFile(ctx, module, ifilename, level+1, "Importing");
}

void Compiler::initContext(Ast::Context& ctx, Ast::Module& module) {
    import(ctx, module, "core/core.ipp", 0);
}

void Compiler::compile() {
    for(Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const std::string& filename = *it;

        std::string ext = getExtention(filename);
        if(_project.zppExt().find(ext) != std::string::npos) {
            Ast::Context ctx(filename);
            Ast::Module module(filename);
            initContext(ctx, module);

            if(!parseFile(ctx, module, filename, 0, "Compiling"))
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

void Compiler::compileString(Ast::Context& ctx, Lexer& lexer, Ast::Module& module, const std::string& data, const int& level, const bool& isEof) {
    Ast::NodeFactory factory(ctx, z::ref(this), module, level);
    lexer.push(factory, data.c_str(), data.size(), isEof);
}

#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/compiler.hpp"
#include "base/ZenlangGenerator.hpp"
#include "base/StlcppGenerator.hpp"

inline bool checkFile(const z::string& filename) {
    return std::ifstream( z::s2e(filename).c_str() ).is_open();
}

inline z::string z::Compiler::findImport(const z::string& filename) {
    z::string cfile;

    // first check current dir
    cfile = "./" + filename;
    if(checkFile(cfile))
        return cfile;

    // next check zen lib dir
    cfile = _project.zlibPath() + "/include/" + filename;
    if(checkFile(cfile))
        return cfile;

    // then all other include paths
    for(z::Ast::Config::PathList::const_iterator it = _config.includePathList().begin(); it != _config.includePathList().end(); ++it) {
        const z::string& dir = *it;
        cfile = dir + "/" + filename;
        if(checkFile(cfile))
            return cfile;
    }
    throw z::Exception("Compiler", zfmt(z::Ast::Token(filename, 0, 0, ""), z::string("Cannot open include file: %{s}").arg("s", filename)));
}

bool z::Compiler::compileFile(z::Ast::Module& module, const z::string& filename, const z::string& msg) {
    if(_project.verbosity() >= z::Ast::Project::Verbosity::Normal) {
        z::string indent = "   ";
        for(z::Ast::Module::Level_t i = 0; i < module.level(); ++i) {
            indent += "  ";
        }
        indent += msg;
    }

    std::ifstream is;
    is.open(z::s2e(filename).c_str(), std::ifstream::in);
    if(is.is_open() == false) {
        throw z::Exception("Compiler", zfmt(z::Ast::Token(filename, 0, 0, ""), z::string("Error opening file %{s}").arg("s", filename)));
    }

    Parser parser;
    Lexer lexer(parser);
    z::Ast::Factory factory(module);
    ParserContext pctx(factory, z::ref(this));
    while(!is.eof()) {
        char buf[1025];
        memset(buf, 0, 1024);
        is.read(buf, 1024);
        std::streamsize got = is.gcount();
        lexer.push(pctx, buf, got, is.eof());
    }
    return true;
}

inline bool z::Compiler::parseFile(z::Ast::Module& module, const z::string& msg) {
    return compileFile(module, module.filename(), msg);
}

void z::Compiler::import(z::Ast::Module& module) {
    z::string ifilename = findImport(module.filename());

    // check if file is already imported
    if(module.unit().headerFileList().find(ifilename) != module.unit().headerFileList().end()) {
        return;
    }

    // if not, add it to list of files imported into this unit
    module.unit().addheaderFile(ifilename);
    compileFile(module, ifilename, "Importing");
}

const z::string ifile =
"typedef void native;\n"

"typedef bool native;\n"

"typedef byte native;\n"
"typedef short native;\n"
"typedef int native;\n"
"typedef long native;\n"

"typedef ubyte native;\n"
"typedef ushort native;\n"
"typedef uint native;\n"
"typedef ulong native;\n"

"typedef float native;\n"
"typedef double native;\n"

"default void = 0;\n"
"default bool = false;\n"

"default byte = 0;\n"
"default short = 0;\n"
"default int = 0;\n"
"default long = 0;\n"

"default ubyte = 0;\n"
"default ushort = 0;\n"
"default uint = 0;\n"
"default ulong = 0;\n"

"default float = 0;\n"
"default double = 0;\n"

"coerce bool => ubyte => byte => ushort => short => uint => int => ulong => long => float => double;\n"

"typedef size native;\n"
"default size = 0;\n"
"coerce int => size;\n"

"typedef char native;\n"
"typedef string native;\n"
"default char = '';\n"
"default string = \"\";\n"
"coerce char => string;\n"

"typedef datetime native;\n"
"default datetime = 0;\n"
"coerce int => datetime;\n"

"typedef type native;\n"

"template <F> future  native;\n"
"template <V> pointer native;\n"
"template <V> value   native;\n"
"template <V> ptr     native;\n"

"template <V>   list native;\n"
"template <K,V> dict native;\n"
"template <K,V> tree native;\n"

"public routine void assert(...) native;\n"
"public routine void unused(...) native;\n"
"public routine void verify(...) native;\n"
"public routine void sizeof(...) native;\n"
"public routine void length(...) native;\n"

"public function (int code)main(const list<string>& argl) abstract;\n"
"public function (int passed)test() abstract;\n"

"struct Config {"
"    enum BuildMode {"
"        Compile;\n"
"        Executable;\n"
"        Shared;\n"
"        Static;\n"
"    };\n"
"    string name;\n"
"    BuildMode buildMode;\n"
"    bool gui;\n"
"    bool dbg;\n"
"    bool isTest;\n"
"    string olang;\n"
"    string apidir;\n"
"    string srcdir;\n"
"    string zexePath;\n"
"    string zlibPath;\n"
"    list<string> includePathList;\n"
"    list<string> includeFileList;\n"
"    list<string> sourceFileList;\n"
"    list<string> linkFileList;\n"
"};\n"

"struct Project {"
"    enum Verbosity {"
"        Silent;\n"
"        Normal;\n"
"        Detailed;\n"
"    };\n"
"    string name;\n"
"    string oproject;\n"
"    Verbosity verbosity;\n"
"    string hppExt;\n"
"    string cppExt;\n"
"    string zppExt;\n"
"    dict<string, Config> configList;\n"
"};\n"
;

void z::Compiler::initContext(z::Ast::Unit& unit) {
    z::Ast::Module module(unit, "core.ipp", 1);
    Parser parser;
    Lexer lexer(parser);
    compileString(module, lexer, ifile, true);
}

void z::Compiler::compile() {
    for(z::Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;
        z::string ext = z::file::getExtention(filename);
        if(_project.zppExt().find(ext) == z::string::npos) {
            continue;
        }
        std::cout << "Compiling " << filename << std::endl;
        z::Ast::Unit unit;
        initContext(unit);

        z::Ast::Module module(unit, filename, 0);
        if(!compileFile(module, filename, "Compiling"))
            throw z::Exception("Compiler", zfmt(Ast::Token(filename, 0, 0, ""), z::string("Cannot open source file: %{s}").arg("s", filename)));

        ZenlangGenerator zgenerator(_project, _config, module);
        zgenerator.run();

        if(_config.olanguage() == "stlcpp") {
            StlcppGenerator generator(_project, _config, module);
            generator.run();
        } else {
            throw z::Exception("Compiler", zfmt(Ast::Token(filename, 0, 0, ""), "Unknown code generator %{s}").arg("s", _config.olanguage()));
        }
    }
}

void z::Compiler::compileString(Ast::Module& module, Lexer& lexer, const z::string& data, const bool& isEof) {
    z::Ast::Factory factory(module);
    ParserContext pctx(factory, z::ref(this));

    const z::estring edata = z::s2e(data);
    lexer.push(pctx, edata.c_str(), edata.size(), isEof);
}

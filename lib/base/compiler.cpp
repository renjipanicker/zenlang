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
    cfile = _project.zlibPath() + "/" + filename;
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
        std::cout << indent << msg << " " << filename << std::endl;
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
    ifilename = z::dir::cleanPath(ifilename);

    // check if file is already imported
    if(module.unit().headerFileList().find(ifilename) != module.unit().headerFileList().end()) {
        return;
    }

    // if not, add it to list of files imported into this unit
    module.unit().addheaderFile(ifilename);
    compileFile(module, ifilename, "Importing");
}

const z::string ifile =
        "namespace z;"
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

        "default byte = 0y;\n"
        "default short = 0s;\n"
        "default int = 0;\n"
        "default long = 0l;\n"

        "default ubyte = 0uy;\n"
        "default ushort = 0us;\n"
        "default uint = 0u;\n"
        "default ulong = 0ul;\n"

        "default float = 0;\n"
        "default double = 0;\n"

        "typedef size native;\n"
        "default size = 0;\n"

        "coerce bool => ubyte => byte => ushort => short => uint => int => ulong => long => size => float => double;\n"
        "coerce size => uint;\n"
        "coerce size => ulong;\n"

        "typedef char native;\n"
        "default char = '';\n"

        "typedef string native;\n"
        "default string = \"\";\n"

        "coerce char => string;\n"

        "struct datetime native;\n"
        "struct data native;\n"
        "struct type native;\n"
        "struct socket native;\n"
        "struct file native;\n"
        "struct widget native;\n"
        "struct window native;\n"

        "template <F> struct future  native;\n"
        "template <V> struct pointer native;\n"
        "template <V> struct value   native;\n"
        "template <V> struct ptr     native;\n"
        "template <V> struct autoptr native;\n"
        "template <V> struct cast    native;\n"

        "template <V>   struct list  native;\n"
        "template <V>   struct olist native;\n"
        "template <V>   struct rlist native;\n"
        "template <V>   struct stack native;\n"
        "template <V>   struct queue native;\n"
        "template <K,V> struct dict  native;\n"
        "template <K,V> struct odict native;\n"
        "template <K,V> struct rdict native;\n"
        "template <K,V> struct tree  native;\n"
        "template <V>   struct map   native;\n"

        "routine void assert(...) native;\n"
        "routine void unused(...) native;\n"
        "routine void verify(...) native;\n"
        "routine size length(...) native;\n"

        "template <V> routine void first(...) native;\n"
        "template <V> routine void last(...) native;\n"

        "template <V> routine void push(...) native;\n"
        "template <V> routine void top(...) native;\n"
        "template <V> routine void pop(...) native;\n"
        "template <V> routine void enqueue(...) native;\n"
        "template <V> routine void dequeue(...) native;\n"
        "template <V> routine void head(...) native;\n"

        "routine void append(...) native;\n"
        "routine void remove(...) native;\n"

        "routine int compare(const string& l, const string& r) native;\n"

        "routine void clear(...) native;\n"
        "routine data raw(...) native;\n"

        "typedef stringlist list<string>;\n"

        "function (int code)main(const stringlist& argl) abstract;\n"
        "function (int passed)test() abstract;\n"
        "function bool device(const int& timeout) abstract;\n"

        "struct SourceFile {"
        "    string filename;\n"
        "};\n"

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
        "    bool isAbstract;\n"
        "    string baseConfig;\n"
        "    string olang;\n"
        "    string pch;\n"
        "    string pchfile;\n"
        "    string apidir;\n"
        "    string srcdir;\n"
        "    string zexePath;\n"
        "    string zlibPath;\n"
        "    list<string> includePathList;\n"
        "    list<string> includeFileList;\n"
        "    list<SourceFile> sourceFileList;\n"
        "    list<SourceFile> guiFileList;\n"
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

void z::Compiler::initContext(z::Ast::Unit& unit, const bool& isprj) {
    // import core.ipp. This is a string constant
    {
        z::Ast::Module module(unit, "core.ipp", 1);
        Parser parser;
        Lexer lexer(parser);
        compileString(module, lexer, ifile, true);
    }

    // import zenlang.ipp. This is a generated file
    if(!isprj) {
        z::Ast::Module module(unit, "zenlang.ipp", 1);
        import(module);
    }
}

void z::Compiler::compile() {
    for(z::Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;
        z::string ext = z::dir::getExtention(filename);
        if(_project.zppExt().find(ext) == z::string::npos) {
            continue;
        }
        std::cout << "Compiling " << filename << std::endl;
        z::Ast::Unit unit;
        initContext(unit, false);

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

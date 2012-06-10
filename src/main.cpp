#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/CmakeGenerator.hpp"
#include "base/MsvcGenerator.hpp"
#include "base/Interpreter.hpp"
#include "base/unit.hpp"
#include "base/compiler.hpp"
#include "base/args.hpp"

static int showHelp(const z::Ast::Config& config) {
    unused(config);
    std::cout << "zen compiler 0.1a";
//    std::cout << " ( at: " << project.zexePath() << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "Copyright(c) 2011 Renji Panicker." << std::endl;
    std::cout << "Usage: zen <options> <files>" << std::endl;
    std::cout << "  -h  --help      Show this message" << std::endl;
    std::cout << "  -i              Interpreter mode" << std::endl;
    std::cout << "  -p              Generate project only" << std::endl;
    std::cout << "  -c              Compile only" << std::endl;
    std::cout << "  -px --exe       Executable project (default)" << std::endl;
    std::cout << "  -pd --dll       Shared library project" << std::endl;
    std::cout << "  -pl --lib       Static library project" << std::endl;
    std::cout << "  -n  --name      Project name" << std::endl;
//    std::cout << "  -g  --gui       GUI application" << std::endl; /// \todo To be kept undocumented.
    std::cout << "  -d  --debug     Debug build" << std::endl;
    std::cout << "  -ol --language  Output language" << std::endl;
    std::cout << "  -op --project   Output project" << std::endl;
    std::cout << "  -ad --api       API directory" << std::endl;
    std::cout << "  -sd --src       Source directory" << std::endl;
    std::cout << "  -v  --verbose   Display verbose output" << std::endl;
    std::cout << "  -t  --test      Don't generate unit tests (Note this is a negative switch)" << std::endl;
    std::cout << "  -z  --zenPath   Zen Library path" << std::endl;
    std::cout << "  -l  --link      Link shared module" << std::endl;
    std::cout << "  -lt  --linkStatic Link static module" << std::endl;
    return 0;
}

inline void initConfig(z::Ast::Project& project, z::Ast::Config& config) {
//    assert(config.includeFileList().size() == 0);
    config.addIncludeFile(config.pch());
    config.addIncludePath(config.apidir());

    // add lib path to include path list
    config.addIncludePath(project.zlibPath());
}

inline void replaceSlash(z::string& path) {
#if defined(WIN32)
    path.replace("\\", "/");
#else
    unused(path);
#endif
}

inline const z::Ast::Expr* resolveTypecast(const z::Ast::Expr& expr) {
    const z::Ast::TypecastExpr* ste = dynamic_cast<const z::Ast::TypecastExpr*>(z::ptr(expr));
    if(ste == 0)
        return z::ptr(expr);
    return resolveTypecast(z::ref(ste).expr());
}

template <typename T>
inline const T& resolveTypeTo(const z::Ast::Expr& expr) {
    const z::Ast::Expr* ex = resolveTypecast(expr);
    assert(ex);
    const T* tex = dynamic_cast<const T*>(ex);
    assert(tex);
    return z::ref(tex);
}

template <typename T>
inline bool canResolveTypeTo(const z::Ast::Expr& expr) {
    const z::Ast::Expr* ex = resolveTypecast(expr);
    assert(ex);
    const T* tex = dynamic_cast<const T*>(ex);
    return(tex != 0);
}

inline const z::string& getStringValue(const z::Ast::Expr& expr) {
    return resolveTypeTo<z::Ast::ConstantStringExpr>(expr).value();
}

inline bool getBoolValue(const z::Ast::Expr& expr) {
    return resolveTypeTo<z::Ast::ConstantBooleanExpr>(expr).value();
}

inline z::stringlist getStringList(const z::Ast::Expr& expr, const z::string& prefix = "") {
    z::stringlist sl;
    const z::Ast::ListExpr& le = resolveTypeTo<z::Ast::ListExpr>(expr);
    for(z::Ast::ListList::List::const_iterator lit = le.list().list().begin(); lit != le.list().list().end(); ++lit) {
        const z::Ast::ListItem& li = lit->get();
        const z::string& val = getStringValue(li.valueExpr());
        sl.add(prefix + val);
    }
    return sl;
}

inline z::stringlist getSourceFileList(const z::Ast::Expr& expr, const z::string& prefix = "") {
    z::stringlist sl;
    const z::Ast::ListExpr& le = resolveTypeTo<z::Ast::ListExpr>(expr);
    for(z::Ast::ListList::List::const_iterator lit = le.list().list().begin(); lit != le.list().list().end(); ++lit) {
        const z::Ast::ListItem& li = lit->get();

        const z::Ast::StructInstanceExpr& sie = resolveTypeTo<z::Ast::StructInstanceExpr>(li.valueExpr());
        const z::string& sname = sie.structDefn().name().string();
        if(sname == "SourceFile") {
            for(z::Ast::StructInitPartList::List::const_iterator it = sie.list().list().begin(); it != sie.list().list().end(); ++it) {
                const z::Ast::StructInitPart& part = it->get();
                const z::string& skey = part.vdef().name().string();
                const z::Ast::Expr& sval = part.expr();
                if(skey == "filename") {
                    const z::string& val = getStringValue(sval);
                    sl.add(prefix + val);
                } else if (skey == "deps") {
                } else {
                    std::cout << "Unknown SourceFile key: " << skey << std::endl;
                }
            }
        } else {
            std::cout << "Unknown struct: " << sname << std::endl;
        }
    }
    return sl;
}

inline void readProjectFile(z::Ast::Project& project, z::Ast::Config& config, const z::string& filename) {
    const z::string path = z::file::getPath(filename);
    z::Ast::Unit unit;
    z::Ast::Module module(unit, filename, 1);
    z::Compiler compiler(project, config);
    compiler.initContext(unit);
    compiler.compileFile(module, filename, "project file");
    for(z::Ast::CompoundStatement::List::const_iterator it = module.globalStatementList().list().begin(); it != module.globalStatementList().list().end(); ++it) {
        const z::Ast::Statement& s = it->get();
        const z::Ast::ExprStatement* exs = dynamic_cast<const z::Ast::ExprStatement*>(z::ptr(s));
        if(exs) {
            const z::Ast::StructInstanceExpr& sie = resolveTypeTo<z::Ast::StructInstanceExpr>(z::ref(exs).expr());
            const z::string& sname = sie.structDefn().name().string();
            if(sname == "Project") {
                for(z::Ast::StructInitPartList::List::const_iterator it = sie.list().list().begin(); it != sie.list().list().end(); ++it) {
                    const z::Ast::StructInitPart& part = it->get();
                    const z::string& skey = part.vdef().name().string();
                    const z::Ast::Expr& sval = part.expr();
                    if(skey == "name") {
                        project.name(getStringValue(sval));
                    } else if (skey == "configList") {
                        const z::Ast::DictExpr& de = resolveTypeTo<z::Ast::DictExpr>(sval);
                        for(z::Ast::DictList::List::const_iterator dit = de.list().list().begin(); dit != de.list().list().end(); ++dit) {
                            const z::Ast::DictItem& di = dit->get();
                            const z::string& ckey = getStringValue(di.keyExpr());
                            const z::Ast::StructInstanceExpr& cvalex = resolveTypeTo<z::Ast::StructInstanceExpr>(di.valueExpr());
                            const z::string& cname = cvalex.structDefn().name().string();
                            if(cname == "Config") {
                                z::Ast::Config& nconfig = project.addConfig(ckey);
                                for(z::Ast::StructInitPartList::List::const_iterator it = cvalex.list().list().begin(); it != cvalex.list().list().end(); ++it) {
                                    const z::Ast::StructInitPart& part = it->get();
                                    const z::string& ckey = part.vdef().name().string();
                                    const z::Ast::Expr& cval = part.expr();
                                    if(ckey == "name") {
                                        nconfig.name(getStringValue(cval));
                                    } else if (ckey == "buildMode") {
                                        const z::Ast::TypeSpecMemberExpr& tse = resolveTypeTo<z::Ast::TypeSpecMemberExpr>(cval);
                                        const z::string& buildMode = tse.vref().name().string();
                                        if(buildMode == "Executable") {
                                            nconfig.buildMode(z::Ast::Config::BuildMode::Executable);
                                        } else if(buildMode == "Shared") {
                                            nconfig.buildMode(z::Ast::Config::BuildMode::Shared);
                                        } else if(buildMode == "Static") {
                                            nconfig.buildMode(z::Ast::Config::BuildMode::Static);
                                        } else {
                                            std::cout << "Unknown build mode: " << buildMode << std::endl;
                                        }
                                    } else if (ckey == "gui") {
                                        nconfig.gui(getBoolValue(cval));
                                    } else if (ckey == "dbg") {
                                        nconfig.debug(getBoolValue(cval));
                                    } else if (ckey == "isTest") {
                                        nconfig.test(getBoolValue(cval));
                                    } else if (ckey == "isAbstract") {
                                        nconfig.abstract(getBoolValue(cval));
                                    } else if (ckey == "baseConfig") {
                                        nconfig.baseConfig(getStringValue(cval));
                                        z::Ast::Config& bc = project.config(nconfig.baseConfig());
                                        nconfig.copy(bc);
                                    } else if (ckey == "olang") {
                                        nconfig.olanguage(getStringValue(cval));
                                    } else if (ckey == "pch") {
                                        nconfig.pch(getStringValue(cval));
                                    } else if (ckey == "pchfile") {
                                        nconfig.pchfile(path + getStringValue(cval));
                                    } else if (ckey == "apidir") {
                                        nconfig.apidir(getStringValue(cval));
                                    } else if (ckey == "srcdir") {
                                        nconfig.srcdir(getStringValue(cval));
                                    } else if (ckey == "includePathList") {
                                        nconfig.addIncludePathList(getStringList(cval, path));
                                    } else if (ckey == "includeFileList") {
                                        nconfig.addIncludeFileList(getStringList(cval, path));
                                    } else if (ckey == "sourceFileList") {
                                        nconfig.addSourceFileList(getSourceFileList(cval, path));
                                    } else if (ckey == "linkFileList") {
                                        nconfig.addLinkFileList(getStringList(cval, path));
                                    } else {
                                        std::cout << "Unknown config key: " << ckey << std::endl;
                                    }
                                }
                                initConfig(project, nconfig);
                            } else {
                                std::cout << "Unknown struct: " << cname << std::endl;
                            }
                        }
                    } else if (skey == "xx") {
                        // place holder for additional project-level properties
                    } else {
                        std::cout << "Unknown project key: " << skey << std::endl;
                    }
                }
            } else {
                std::cout << "Unknown struct: " << sname << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    z::Application app(argc, argv);

    z::Ast::Project project;
    z::Ast::Config& config = project.addConfig("cmd");
    config.abstract(true);

    z::string p = z::app().path();
    replaceSlash(p);
    project.zexePath(p);
    config.addLinkFile("core");

    if (argc < 2) {
        return showHelp(config);
    }

    bool interpreterMode = false;
    int i = 1;
    z::string ptype;
    while(i < argc) {
        z::string t = argv[i++];
        if((t == "-h") || (t == "--help")) {
            return showHelp(config);
        } else if(t == "-c") {
            config.buildMode(z::Ast::Config::BuildMode::Compile);
        } else if((t == "-p") || (t == "--project")) {
            t = argv[i++];
            ptype = t;
        } else if(t == "-i") {
            interpreterMode = true;
        } else if((t == "-n") || (t == "--name")) {
            t = argv[i++];
            project.name(t);
        } else if((t == "-px") || (t == "--exe")) {
            config.buildMode(z::Ast::Config::BuildMode::Executable);
        } else if((t == "-pd") || (t == "--dll")) {
            config.buildMode(z::Ast::Config::BuildMode::Shared);
        } else if((t == "-pl") || (t == "--lib")) {
            config.buildMode(z::Ast::Config::BuildMode::Static);
        } else if((t == "-g") || (t == "--gui")) {
            config.gui(true);
        } else if((t == "-d") || (t == "--debug")) {
            config.debug(true);
        } else if((t == "-ip") || (t == "--inputProject")) {
            t = argv[i++];
            readProjectFile(project, config, t);
        } else if((t == "-ol") || (t == "--olanguage")) {
            t = argv[i++];
            config.olanguage(t);
        } else if((t == "-op") || (t == "--oproject")) {
            t = argv[i++];
            project.oproject(t);
        } else if((t == "-ad") || (t == "--api")) {
            t = argv[i++];
            replaceSlash(t);
            config.apidir(t);
        } else if((t == "-sd") || (t == "--src")) {
            t = argv[i++];
            replaceSlash(t);
            config.srcdir(t);
        } else if((t == "-ph") || (t == "--pch")) {
            t = argv[i++];
            config.pch(t);
        } else if((t == "-pf") || (t == "--pchfile")) {
            t = argv[i++];
            config.pchfile(t);
        } else if((t == "-l") || (t == "--link")) {
            t = argv[i++];
            config.addLinkFile(t);
            if(t == "gui") {
                config.gui(true);
            }
        } else if((t == "-lt") || (t == "--linkStatic")) {
            t = argv[i++];
        } else if((t == "-v") || (t == "--verbose")) {
            project.verbosity(z::Ast::Project::Verbosity::Detailed);
        } else if((t == "-t") || (t == "--test")) {
            config.test(false);
        } else if((t == "-z") || (t == "--zenPath")) {
            t = argv[i++];
            replaceSlash(t);
            project.zlibPath(t);
        } else {
            replaceSlash(t);
            config.addSourceFile(t);
        }
    }

    if(project.zlibPath().size() == 0) {
        // get path component of exe-path
        z::string p = project.zexePath();
        // strip path, if any
        char sep = '/';
        z::string::size_type idx = p.rfind(sep);
        if(idx != z::string::npos) {
            p = p.substr(0, idx);
            /*
            z::string::size_type idx = p.rfind(sep);
            if(idx != z::string::npos) {
                p = p.substr(0, idx);
            } else {
                p = ".";
                p += sep;
            }
            */
#if defined(__APPLE__)
            //p += "/lib";
#endif
        } else {
            p = ".";
            p += sep;
        }
        
        project.zlibPath(p);
    }

    initConfig(project, config);

    if(project.verbosity() == z::Ast::Project::Verbosity::Detailed) {
        z::string cwd = z::file::cwd();
        std::cout << "cwd: " << cwd << std::endl;
        std::cout << "exe: " << project.zexePath() << std::endl;
        std::cout << "lib: " << project.zlibPath() << std::endl;
        std::cout << "api: " << config.apidir() << std::endl;
        std::cout << "src: " << config.srcdir() << std::endl;
        if(config.includePathList().size() > 0) {
            std::cout << "inc: ";
            for (z::Ast::Config::PathList::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
                const z::string& p = *it;
                std::cout << p << ";";
            }
            std::cout << std::endl;
        }
    }

    if(interpreterMode) {
        z::Interpreter intp(project, config);
        intp.run();
    } else {
        if(project.oproject() == "cmake") {
#if defined(WIN32)
            z::MsvcGenerator progen(project);
            //z::CmakeGenerator progen(project);
#else
                z::CmakeGenerator progen(project);
#endif
            progen.run();
        } else {
            throw z::Exception("Main", z::string("Unknown project generator %{s}").arg("s", project.oproject()));
        }
    }
    return 0;
}

// dummy function
void z::Application::onExit() {
}

#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "CmakeGenerator.hpp"
#include "NodeFactory.hpp"
#include "Interpreter.hpp"

static int showHelp(const Ast::Config& config) {
    std::cout << "zen compiler 0.1a";
    std::cout << " (" << config.zexePath().c_str() << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "Copyright(c) 2011 Renji Panicker." << std::endl;
    std::cout << "Usage: zen <options> <files>" << std::endl;
    std::cout << "  -h  --help      Show this message" << std::endl;
    std::cout << "  -i              Interpreter mode" << std::endl;
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
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;
    Ast::Config& config = project.addConfig("");

    static const int len = 1024;
    char path[len] = "";
#ifdef WIN32
    DWORD rv = GetModuleFileName(NULL, path, len);
    if(rv == 0) {
        DWORD ec = GetLastError();
        assert(ec != ERROR_SUCCESS);
        std::cout << "Internal error retreiving process path " << ec << std::endl;
        return -1;
    }
#else
    if (readlink ("/proc/self/exe", path, len) == -1) {
        std::cout << "Internal error retreiving process path " << path << std::endl;
        return -1;
    }
#endif

    config.zexePath(path);
    config.addIncludeFile("base/pch.hpp");
    config.addIncludeFile("base/zenlang.hpp");
    config.addLinkFile("core");

    if (argc < 2) {
        return showHelp(config);
    }

    bool interpreterMode = false;
    int i = 1;
    while(i < argc) {
        z::string t = argv[i++];
        if((t == "-h") || (t == "--help")) {
            return showHelp(config);
        } else if(t == "-c") {
            config.buildMode(Ast::Config::BuildMode::Compile);
        } else if(t == "-i") {
            interpreterMode = true;
        } else if((t == "-n") || (t == "--name")) {
            t = argv[i++];
            project.name(t);
        } else if((t == "-px") || (t == "--exe")) {
            config.buildMode(Ast::Config::BuildMode::Executable);
        } else if((t == "-pd") || (t == "--dll")) {
            config.buildMode(Ast::Config::BuildMode::Shared);
        } else if((t == "-pl") || (t == "--lib")) {
            config.buildMode(Ast::Config::BuildMode::Static);
        } else if((t == "-g") || (t == "--gui")) {
            config.gui(true);
        } else if((t == "-d") || (t == "--debug")) {
            config.debug(true);
        } else if((t == "-ol") || (t == "--olanguage")) {
            t = argv[i++];
            config.olanguage(t);
        } else if((t == "-op") || (t == "--oproject")) {
            t = argv[i++];
            project.oproject(t);
        } else if((t == "-ad") || (t == "--api")) {
            t = argv[i++];
            config.apidir(t);
        } else if((t == "-sd") || (t == "--src")) {
            t = argv[i++];
            config.srcdir(t);
        } else if((t == "-l") || (t == "--link")) {
            t = argv[i++];
            config.addLinkFile(t);
            if(t == "gui") {
                config.gui(true);
            }
//      } else if((t == "-lt") || (t == "--linkStatic")) {
//          t = argv[i++];
//      } else if((t == "-lh") || (t == "--linkShared")) {
//          t = argv[i++];
        } else if((t == "-v") || (t == "--verbose")) {
            project.verbosity(Ast::Project::Verbosity::Detailed);
        } else if((t == "-t") || (t == "--test")) {
            config.test(false);
        } else if((t == "-z") || (t == "--zenPath")) {
            t = argv[i++];
            config.zlibPath(t);
        } else {
            config.addSourceFile(t);
        }
    }

    if(config.zlibPath().size() == 0) {
        z::string p = config.zexePath();
        // strip path, if any
#ifdef WIN32
        char sep = '\\';
#else
        char sep = '/';
#endif
        z::string::size_type idx = p.rfind(sep);
        if(idx != z::string::npos) {
            p = p.substr(0, idx);
            z::string::size_type idx = p.rfind(sep);
            if(idx != z::string::npos) {
                p = p.substr(0, idx);
            } else {
                p = ".";
                p += sep;
            }
        } else {
            p = ".";
            p += sep;
        }

        p.replace("\\", "/");
        config.zlibPath(p);
    }

    if(project.verbosity() == Ast::Project::Verbosity::Detailed) {
        z::string cwd = z::file::cwd();
        std::cout << "cwd: " << cwd << std::endl;
        std::cout << "exe: " << config.zexePath() << std::endl;
        std::cout << "lib: " << config.zlibPath() << std::endl;
        std::cout << "api: " << config.apidir() << std::endl;
        std::cout << "src: " << config.srcdir() << std::endl;
    }

    if(interpreterMode) {
        Interpreter intp(project, config);
        intp.run();
    } else {
        if(project.oproject() == "cmake") {
            CmakeGenerator progen(project);
            progen.run();
        } else {
            throw z::Exception("Main", z::fmt("Unknown project generator %{s}").add("s", project.oproject()));
        }
    }
    return 0;
}

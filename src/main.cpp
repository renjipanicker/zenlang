#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "progen.hpp"

static int showHelp(const Ast::Config& config) {
    fprintf(stdout, "zen compiler 0.1a");
    fprintf(stdout, " (%s)", config.zexePath().c_str());
    fprintf(stdout, "\n");
    fprintf(stdout, "Copyright(c) 2011 Renji Panicker.\n");
    fprintf(stdout, "Usage: zen <options> <files>\n");
    fprintf(stdout, "  -h  --help      Show this message\n");
    fprintf(stdout, "  -c              Compile only\n");
    fprintf(stdout, "  -px --exe       Executable project (default)\n");
    fprintf(stdout, "  -pd --dll       Shared library project\n");
    fprintf(stdout, "  -pl --lib       Static library project\n");
    fprintf(stdout, "  -n  --name      Project name\n");
    fprintf(stdout, "  -g  --gui       GUI application\n");
    fprintf(stdout, "  -d  --debug     Debug build\n");
    fprintf(stdout, "  -t  --test      Don't generate unit tests (Note this is a negative switch)\n");
    fprintf(stdout, "  -z  --zenPath   Zen Library path\n");
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;
    Ast::Config& config = project.addConfig("");

    static const int len = 1024;
    char path[len];
#ifdef WIN32
    GetModuleFileName(NULL, path, len);
    if(GetLastError() != ERROR_SUCCESS) {
        fprintf(stdout, "Internal error retreiving process path\n");
        return -1;
    }
#else
    if (readlink ("/proc/self/exe", path, len) == -1) {
        fprintf(stdout, "Internal error retreiving process path %s\n", path);
        return -1;
    }
#endif

    config.zexePath(path);
    config.addIncludeFile("base/pch.hpp");
    config.addIncludeFile("base/zenlang.hpp");

    if (argc < 2) {
        return showHelp(config);
    }

    int i = 1;
    while(i < argc) {
        std::string t = argv[i++];
        if((t == "-h") || (t == "--help")) {
            return showHelp(config);
        } else if(t == "-c") {
            config.mode(Ast::Config::Mode::Compile);
        } else if((t == "-n") || (t == "--name")) {
            t = argv[i++];
            project.name(t);
        } else if((t == "-px") || (t == "--exe")) {
            config.mode(Ast::Config::Mode::Executable);
        } else if((t == "-pd") || (t == "--dll")) {
            config.mode(Ast::Config::Mode::Shared);
        } else if((t == "-pl") || (t == "--lib")) {
            config.mode(Ast::Config::Mode::Static);
        } else if((t == "-g") || (t == "--gui")) {
            config.gui(true);
        } else if((t == "-d") || (t == "--debug")) {
            config.debug(true);
        } else if((t == "-t") || (t == "--test")) {
            config.test(false);
        } else if((t == "-z") || (t == "--zenPath")) {
            t = argv[i++];
            config.zlibPath(t);
        } else {
            config.addSourceFile(t);
        }
    }

    ProGen progen(project);
    progen.run();
    return 0;
}

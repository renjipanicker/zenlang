#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"

static int showHelp(const Ast::Project& project) {
    fprintf(stdout, "zen compiler 0.1a");
    fprintf(stdout, " (%s)", project.zexePath().c_str());
    fprintf(stdout, "\n");
    fprintf(stdout, "Copyright(c) 2011 Renji Panicker.\n");
    fprintf(stdout, "Usage: zen <options> <files>\n");
    fprintf(stdout, "  -h --help      Show this message\n");
    fprintf(stdout, "  -c             Compile only\n");
    fprintf(stdout, "  -n --name      Project name\n");
    fprintf(stdout, "  -z --zenPath   Zen Library path\n");
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;

    static const int len = 1024;
    char path[len];
#ifdef WIN32
    GetModuleFileName(NULL, path, len);
    if(GetLastError() != ERROR_SUCCESS) {
        fprintf(stdout, "Internal error retreiving process path\n");
        return -1;
    }
#else
    if (readlink ("/proc/self/exe", path, len) != -1) {
        fprintf(stdout, "Internal error retreiving process path\n");
        return -1;
    }
#endif
    project.zexePath(path);

    project.global().addIncludeFile("base/pch.hpp");
    project.global().addIncludeFile("base/zenlang.hpp");

    if (argc < 2) {
        return showHelp(project);
    }

    int i = 1;
    while(i < argc) {
        std::string t = argv[i++];
        if((t == "-h") || (t == "--help")) {
            return showHelp(project);
        } else if(t == "-c") {
            project.mode(Ast::Project::Mode::Compile);
        } else if((t == "-n") || (t == "--name")) {
            t = argv[i++];
            project.name(t);
        } else if((t == "-z") || (t == "--zenPath")) {
            t = argv[i++];
            project.zlibPath(t);
        } else {
            project.global().addSourceFile(t);
        }
    }

    Compiler compiler(project);
    compiler.compile();
    return 0;
}

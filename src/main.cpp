#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "CmakeGenerator.hpp"
#include "NodeFactory.hpp"
#include "Interpreter.hpp"

static int showHelp(const Ast::Config& config) {
    printf("zen compiler 0.1a");
    printf(" (%s)", config.zexePath().c_str());
    printf("\n");
    printf("Copyright(c) 2011 Renji Panicker.\n");
    printf("Usage: zen <options> <files>\n");
    printf("  -h  --help      Show this message\n");
    printf("  -i              Interpreter mode\n");
    printf("  -c              Compile only\n");
    printf("  -px --exe       Executable project (default)\n");
    printf("  -pd --dll       Shared library project\n");
    printf("  -pl --lib       Static library project\n");
    printf("  -n  --name      Project name\n");
    printf("  -g  --gui       GUI application\n");
    printf("  -d  --debug     Debug build\n");
    printf("  -ol --language  Output language\n");
    printf("  -op --project   Output project\n");
    printf("  -ad --api       API directory\n");
    printf("  -sd --src       Source directory\n");
    printf("  -v  --verbose   Display verbose output\n");
    printf("  -t  --test      Don't generate unit tests (Note this is a negative switch)\n");
    printf("  -z  --zenPath   Zen Library path\n");
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;
    Ast::Config& config = project.addConfig("");

    static const int len = 1024;
    char path[len] = "";
#ifdef WIN32
    GetModuleFileName(NULL, path, len);
    if(GetLastError() != ERROR_SUCCESS) {
        printf("Internal error retreiving process path\n");
        return -1;
    }
#else
    if (readlink ("/proc/self/exe", path, len) == -1) {
        printf("Internal error retreiving process path %s\n", path);
        return -1;
    }
#endif

    config.zexePath(path);
    config.addIncludeFile("base/pch.hpp");
    config.addIncludeFile("base/zenlang.hpp");

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
            config.mode(Ast::Config::Mode::Compile);
        } else if(t == "-i") {
            interpreterMode = true;
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

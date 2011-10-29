#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"

static int showHelp() {
    fprintf(stdout, "zen compiler 0.1\n");
    fprintf(stdout, "Copyright(c) 2011 Renji Panicker.\n");
    fprintf(stdout, "zen <options> <files>\n");
    fprintf(stdout, "  -h --help Show this message\n");
    fprintf(stdout, "  -c        Compile only\n");
    fprintf(stdout, "  -n --name Project name\n");
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;
    project.global().addIncludePath(".");
    project.global().addIncludePath("../../zenlang/lib");
    project.global().addIncludeFile("base/pch.hpp");
    project.global().addIncludeFile("base/zenlang.hpp");

    if (argc < 2) {
        return showHelp();
    }

    int i = 1;
    while(i < argc) {
        std::string t = argv[i++];
        if((t == "-h") || (t == "--help")) {
            return showHelp();
        } else if(t == "-c") {
            project.mode(Ast::Project::Mode::Compile);
        } else if((t == "-n") || (t == "--name")) {
            t = argv[i++];
            project.name(t);
        } else {
            project.global().addSourceFile(t);
        }
    }

    Compiler compiler(project);
    compiler.compile();
    return 0;
}

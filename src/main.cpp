#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"

static std::string app;

static int showHelp() {
    fprintf(stderr, "%s <file>\n", app.c_str());
    return 0;
}

int main(int argc, char* argv[]) {
    Ast::Project project;
    project.addInclude(".");
    project.addInclude("../../zenlang/lib");
    project.addIncludeFile("base/pch.hpp");
    project.addIncludeFile("base/zenlang.hpp");

    int i = 0;
    if (argc < 2) {
        return showHelp();
    }

    app = argv[i++];
    while(i < argc) {
        std::string t(argv[i++]);
        if((t == "-h") || (t == "--help")){
            return showHelp();
        } else if(t == "-c") {
            project.mode(Ast::Project::Mode::Compile);
        } else {
            project.addSource(t);
        }
    }

    Compiler compiler(project);
    compiler.compile();
    return 0;
}

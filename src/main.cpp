#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "compiler.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return 1;;
    }

    Project project;
    project.addInclude(".");
    project.addInclude("../../zenlang/lib");
    project.addIncludeFile("base/pch.hpp");
    project.addIncludeFile("base/common.hpp");
    project.addIncludeFile("base/exception.hpp");
    project.addSource(argv[1]);

    Compiler compiler(project);
    compiler.compile();
    return 0;
}

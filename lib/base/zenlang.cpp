#include "pch.hpp"
#include "zenlang.hpp"

static TestInstance* g_testListHead = 0;
static TestInstance* g_testListTail = 0;

TestInstance::TestInstance() : _next(0) {
    if(g_testListTail == 0) {
        assert(g_testListHead == 0);
        g_testListHead = this;
    } else {
        ref(g_testListTail)._next = this;
    }
    g_testListTail = this;
}

CallContext g_context = CallContext();
CallContext& CallContext::get() {
    return g_context;
}

void CallContext::run() {
    while(_list.size() > 0) {
        FunctionList::Ptr ptr;
        if(_list.pop(ptr))
            ptr->run();
    }
}

#if defined(Z_EXE)
int main(int argc, char* argv[]) {
    g_testListTail = g_testListHead;
    while(g_testListTail != 0) {
        ref(g_testListTail).enque(g_context);
        g_testListTail = ref(g_testListTail)._next;
    }

    std::list<std::string> argl;
    Main(argl);
    g_context.run();
    return 0;
}
#endif

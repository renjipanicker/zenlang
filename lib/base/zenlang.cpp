#include "pch.hpp"
#include "zenlang.hpp"

CallContext g_context = CallContext();

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

void CallContext::run() {
    while(_invocationList.size() > 0) {
        Invocation* invocation = _invocationList.front();
        _invocationList.pop_front();
        ref(invocation).run();
        delete invocation;
    }
}

int main(int argc, char* argv[]) {
    g_testListTail = g_testListHead;
    while(g_testListTail != 0) {
        ref(g_testListTail).enque(g_context);
        g_testListTail = ref(g_testListTail)._next;
    }

    g_context.run();
    return 0;
}
